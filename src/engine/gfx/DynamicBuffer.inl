#pragma once
#include <glad/glad.h>
#include <glm/vec3.hpp>

namespace GFX
{
  template<typename UserT>
  DynamicBuffer<UserT>::DynamicBuffer(uint32_t size, uint32_t alignment)
    : align_(alignment), capacity_(size)
  {
    // align
    size += (align_ - (size % align_)) % align_;

    // allocate uninitialized memory in VRAM
    buffer = std::make_unique<StaticBuffer>(nullptr, size);

    // make one big null allocation
    allocationData<UserT> phalloc;
    phalloc.handle = NULL;
    phalloc.offset = 0;
    phalloc.size = size;
    phalloc.time = 0;
    allocs_.push_back(phalloc);
  }


  template<typename UserT>
  uint64_t DynamicBuffer<UserT>::Allocate(const void* data, size_t size, UserT userdata)
  {
    size += (align_ - (size % align_)) % align_;
    // find smallest NULL allocation that will fit
    Iterator small = allocs_.end();
    for (int i = 0; i < allocs_.size(); i++)
    {
      if (allocs_[i].handle == NULL && allocs_[i].size >= size) // potential allocation
      {
        if (small == allocs_.end())// initialize small
          small = allocs_.begin() + i;
        else if (allocs_[i].size < small->size)
          small = allocs_.begin() + i;
      }
    }
    // allocation failure
    if (small == allocs_.end())
      return NULL;

    // split free allocation
    allocationData<UserT> newAlloc(userdata);
    newAlloc.handle = nextHandle++;
    newAlloc.offset = small->offset;
    newAlloc.size = size;
    newAlloc.time = timer.Elapsed();
    newAlloc.flags = 0;
    //newAlloc.userdata = userdata;

    small->offset += newAlloc.size;
    small->size -= newAlloc.size;

    // replace shrunk alloc if it would become degenerate
    if (small->size == 0)
      *small = newAlloc;
    else
      allocs_.insert(small, newAlloc);

    buffer->SubData(data, newAlloc.size, newAlloc.offset);
    //glNamedBufferSubData(gpuHandle, newAlloc.offset, newAlloc.size, data);
    ++numActiveAllocs_;
    stateChanged();
    return newAlloc.handle;
  }


  template<typename UserT>
  bool DynamicBuffer<UserT>::Free(uint64_t handle)
  {
    if (handle == NULL) return false;
    auto it = std::find_if(allocs_.begin(), allocs_.end(), [&](const auto& a) { return a.handle == handle; });
    if (it == allocs_.end()) // failed to free
      return false;

    it->handle = NULL;
    maybeMerge(it);
    --numActiveAllocs_;
    stateChanged();
    return true;
  }


  template<typename UserT>
  uint64_t DynamicBuffer<UserT>::FreeOldest()
  {
    // find and free the oldest allocation
    Iterator old = allocs_.end();
    for (int i = 0; i < allocs_.size(); i++)
    {
      if (allocs_[i].handle != NULL)
      {
        if (old == allocs_.end())
          old = allocs_.begin() + i;
        else if (allocs_[i].time < old->time)
          old = allocs_.begin() + i;
      }
    }

    // failed to find old node to free
    if (old == allocs_.end())
      return NULL;

    auto retval = old->handle;
    old->handle = NULL;
    maybeMerge(old);
    --numActiveAllocs_;
    stateChanged();
    return retval;
  }


  template<typename UserT>
  inline void DynamicBuffer<UserT>::stateChanged()
  {
    //DEBUG_DO(dbgVerify());
  }

  template<typename UserT>
  void DynamicBuffer<UserT>::maybeMerge(Iterator it)
  {
    bool removeIt = false;
    bool removeNext = false;

    // merge with next alloc
    if (it != allocs_.end() - 1)
    {
      Iterator next = it + 1;
      if (next->handle == NULL)
      {
        it->size += next->size;
        removeNext = true;
      }
    }

    // merge with previous alloc
    if (it != allocs_.begin())
    {
      Iterator prev = it - 1;
      if (prev->handle == NULL)
      {
        prev->size += it->size;
        removeIt = true;
      }
    }

    // erase merged allocations
    if (removeIt && removeNext)
      allocs_.erase(it, it + 2); // this and next
    else if (removeIt)
      allocs_.erase(it);         // just this
    else if (removeNext)
      allocs_.erase(it + 1);     // just next
  }


  template<typename UserT>
  void DynamicBuffer<UserT>::dbgVerify()
  {
    uint64_t prevPtr = 1;
    GLsizei sumSize = 0;
    GLuint active = 0;
    for (const auto& alloc : allocs_)
    {
      if (alloc.handle != NULL)
        active++;
      // check there are never two null blocks in a row
      ASSERT_MSG(!(prevPtr == NULL && alloc.handle == NULL),
        "Verify failed: two null blocks in a row!");
      prevPtr = alloc.handle;

      // check offset is equal to total size so far
      ASSERT_MSG(alloc.offset == sumSize,
        "Verify failed: size/offset discrepancy!");
      sumSize += alloc.size;

      // check alignment
      ASSERT_MSG(alloc.offset % align_ == 0,
        "Verify failed: block alignment mismatch!");

      // check degenerate (0-size) allocation
      ASSERT_MSG(alloc.size != 0,
        "Verify failed: 0-size allocation!");
    }

    ASSERT_MSG(active == numActiveAllocs_,
      "Verify failed: active allocations mismatch!");
  }


  template<typename UserT>
  inline void DebugDrawableBuffer<UserT>::GenDrawData()
  {
    const glm::vec3 free_color{ .2, 1, 1 };  // light cyan
    const glm::vec3 full_color{ 1, .3, .3 }; // light red
    bool alternator = true;

    if (!vao_)
    {
      glCreateVertexArrays(1, &vao_);
      glBindVertexArray(vao_);
      glEnableVertexAttribArray(0);
      glEnableVertexAttribArray(1);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
      glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    }

    std::vector<glm::vec3> data;
    for (const auto& alloc : this->allocs_)
    {
      glm::vec3 color = alloc.handle == NULL ? free_color : full_color;
      if (color == free_color)
        alternator = true;
      color *= (alternator ? 1.f : .5f);

      // position 1
      data.push_back({ (float)alloc.offset / (float)this->capacity_, 0, 0 });
      // color 1
      data.push_back(color);

      // position 2
      data.push_back({ (float)(alloc.offset + alloc.size) / (float)this->capacity_, 0, 0 });
      // color 2
      data.push_back(color);

      alternator = !alternator;
    }

    vbo_ = std::make_unique<StaticBuffer>(&data[0][0], sizeof(glm::vec3) * data.size());
    vbo_->Bind<GFX::Target::VERTEX_BUFFER>();
  }


  // length is hardcoded, line width set beforehand by user
  template<typename UserT>
  inline void DebugDrawableBuffer<UserT>::Draw()
  {
    if (vao_)
    {
      int dataSize = this->allocs_.size() * 4;
      glBindVertexArray(vao_);
      glDrawArrays(GL_LINES, 0, dataSize / 2);
    }
  }


  //template<typename UserT>
  //inline void DynamicBuffer<UserT>::Update()
  //{
  //  if (dirty_)
  //  {
  //    //genDrawData();
  //    bufferAllocData();
  //    dirty_ = false;
  //  }
  //}
}