#pragma once
#include <stdint.h>
#include <Utilities/Timer.h>
#include <CoreEngine/GraphicsIncludes.h>

namespace GFX
{
  class StaticBuffer;

  // signifies no userdata specialization
  struct None_ {};

  // Generic GPU buffer that can store
  //   up to 4GB (UINT32_MAX) of data
  template<typename UserT = None_>
  class DynamicBuffer
  {
  public:
    DynamicBuffer(uint32_t size, uint32_t alignment);
    ~DynamicBuffer() {};

    // allocates a chunk of memory in the data store, returns handle to memory
    // the handle is used to free the chunk when the user is done with it
    uint64_t Allocate(const void* data, size_t size, UserT userdata = {});

    // frees a chunk of memory being "pointed" to by a handle
    // returns true if the memory was able to be freed, false otherwise
    bool Free(uint64_t handle);

    // frees the oldest allocated chunk
    // returns handle to freed chunk, 0 if nothing was freed
    uint64_t FreeOldest();

    // query information about the allocator
    const auto& GetAlloc(uint64_t handle) { return *std::find_if(allocs_.begin(), allocs_.end(), [=](const auto& alloc) { return alloc.handle == handle; }); }
    const auto& GetAllocs() { return allocs_; }
    GLuint ActiveAllocs() { return numActiveAllocs_; }
    GLuint GetGPUHandle() { return buffer->GetID(); }

    // compare return values of this func to see if the state has change
    std::pair<uint64_t, GLuint> GetStateInfo() { return { nextHandle, numActiveAllocs_ }; }

    const GLsizei align_; // allocation alignment

    template<typename UT = UserT>
    struct allocationData
    {
      allocationData() = default;
      allocationData(UT u) : userdata(u) {}

      uint64_t handle{}; // "pointer"
      double time{};     // time of allocation
      uint32_t flags{};  // GPU flags
      uint32_t _pad{};   // GPU padding
      uint32_t offset{}; // offset from beginning of this memory
      uint32_t size{};   // allocation size
      UT userdata{};     // user-defined data
    };

    template<>
    struct allocationData<None_>
    {
      allocationData() = default;
      allocationData(None_) {}; // semicolon to make intellisense stop complaining

      uint64_t handle{}; // "pointer"
      double time{};     // time of allocation
      uint32_t flags{};  // GPU flags
      uint32_t _pad{};   // GPU padding
      uint32_t offset{}; // offset from beginning of this memory
      uint32_t size{};   // allocation size
    };

    size_t AllocSize() const { return sizeof(allocationData<UserT>); }

  protected:
    std::vector<allocationData<UserT>> allocs_;
    using Iterator = decltype(allocs_.begin());

    // called whenever anything about the allocator changed
    void stateChanged();

    // merges null allocations adjacent to iterator
    void maybeMerge(Iterator it);

    // verifies the buffer has no errors, debug only
    void dbgVerify();

    std::unique_ptr<StaticBuffer> buffer;
    uint64_t nextHandle = 1;
    GLuint numActiveAllocs_ = 0;
    const GLuint capacity_; // for fixed size buffers
    Timer timer;
  };

  template<typename UserT>
  class DebugDrawableBuffer : public DynamicBuffer<UserT>
  {
  public:
    void Draw();
    void GenDrawData();

  private:
    GLuint vao_{};
    std::unique_ptr<StaticBuffer> vbo_;
  };
}

#include "DynamicBuffer.inl"