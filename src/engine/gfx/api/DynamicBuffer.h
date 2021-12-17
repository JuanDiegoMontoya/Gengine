#pragma once
#include <stdint.h>
#include <utility/Timer.h>
#include "Buffer.h"
#include "../../GAssert.h"

namespace GFX
{
  //class ManagedBufferBase
  //{
  //public:
  //  ManagedBufferBase(uint32_t size, uint32_t alignment);
  //  ~ManagedBufferBase() {};

  //  // allocates a chunk of memory in the data store, returns handle to memory
  //  // the handle is used to free the chunk when the user is done with it
  //  uint64_t Allocate(const void* data, size_t size, UserT userdata = {});

  //  // frees a chunk of memory being "pointed" to by a handle
  //  // returns true if the memory was able to be freed, false otherwise
  //  bool Free(uint64_t handle);

  //  // frees the oldest allocated chunk
  //  // returns handle to freed chunk, 0 if nothing was freed
  //  uint64_t FreeOldest();

  //  // query information about the allocator
  //  const auto& GetAlloc(uint64_t handle) { return *std::find_if(allocs_.begin(), allocs_.end(), [=](const auto& alloc) { return alloc.handle == handle; }); }
  //  const auto& GetAllocs() { return allocs_; }
  //  uint32_t ActiveAllocs() { return numActiveAllocs_; }
  //  uint32_t GetID() { return buffer->GetID(); }
  //  uint32_t GetAllocOffset(uint64_t handle) { for (uint32_t i = 0; i < allocs_.size(); i++) if (handle == allocs_[i].handle) return i; UNREACHABLE; }

  //  // compare return values of this func to see if the state has change
  //  std::pair<uint64_t, uint32_t> GetStateInfo() { return { nextHandle, numActiveAllocs_ }; }

  //  const size_t align_; // allocation alignment

  //  template<typename UT = UserT>
  //  struct allocationData
  //  {
  //    allocationData() = default;
  //    allocationData(UT u) : userdata(u) {}

  //    uint64_t handle{}; // "pointer"
  //    double time{};     // time of allocation
  //    uint32_t flags{};  // GPU flags
  //    uint32_t _pad{};   // GPU padding
  //    uint32_t offset{}; // offset from beginning of this memory
  //    uint32_t size{};   // allocation size
  //    UT userdata{};     // user-defined data
  //  };

  //  template<>
  //  struct allocationData<None_>
  //  {
  //    allocationData() = default;
  //    allocationData(None_) {}; // semicolon to make intellisense stop complaining

  //    uint64_t handle{}; // "pointer"
  //    double time{};     // time of allocation
  //    uint32_t flags{};  // GPU flags
  //    uint32_t _pad{};   // GPU padding
  //    uint32_t offset{}; // offset from beginning of this memory
  //    uint32_t size{};   // allocation size
  //  };

  //  size_t AllocSize() const { return sizeof(allocationData<UserT>); }

  //protected:
  //  std::vector<allocationData<UserT>> allocs_;
  //  using Iterator = decltype(allocs_.begin());

  //  // called whenever anything about the allocator changed
  //  void stateChanged();

  //  // merges null allocations adjacent to iterator
  //  void maybeMerge(Iterator it);

  //  // verifies the buffer has no errors, debug only
  //  void dbgVerify();

  //  std::unique_ptr<Buffer> buffer;
  //  uint64_t nextHandle = 1;
  //  uint32_t numActiveAllocs_ = 0;
  //  const uint32_t capacity_; // for fixed size buffers
  //  Timer timer;
  //};

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
    uint32_t ActiveAllocs() { return numActiveAllocs_; }
    uint32_t GetID() { return buffer->GetID(); }
    uint32_t GetAllocOffset(uint64_t handle) { for (uint32_t i = 0; i < allocs_.size(); i++) if (handle == allocs_[i].handle) return i; UNREACHABLE; }

    // compare return values of this func to see if the state has change
    std::pair<uint64_t, uint32_t> GetStateInfo() { return { nextHandle, numActiveAllocs_ }; }

    const size_t align_; // allocation alignment

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

    std::unique_ptr<Buffer> buffer;
    uint64_t nextHandle = 1;
    uint32_t numActiveAllocs_ = 0;
    const uint32_t capacity_; // for fixed size buffers
    Timer timer;
  };

  template<typename UserT>
  class DebugDrawableBuffer : public DynamicBuffer<UserT>
  {
  public:
    DebugDrawableBuffer(uint32_t size, uint32_t alignment)
      : DynamicBuffer<UserT>(size, alignment)
    {
    }

    void Draw();
    void GenDrawData();

  private:
    uint32_t vao_{};
    std::unique_ptr<Buffer> vbo_;
  };
}

#include "DynamicBuffer.cpp"