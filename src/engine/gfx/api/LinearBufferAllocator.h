#pragma once
#include <optional>
#include <cstdint>
#include <span>

namespace GFX
{
  class Buffer;

  class LinearBufferAllocator
  {
  public:
    LinearBufferAllocator() = delete;
    LinearBufferAllocator(const LinearBufferAllocator&) = delete;
    LinearBufferAllocator& operator=(const LinearBufferAllocator&) = delete;

    static std::optional<LinearBufferAllocator> Create(Buffer* backingBuffer);

    // returns a byte offset in the buffer
    template<typename T>
    size_t Allocate(std::span<T> data, size_t alignment)
    {
      return Allocate(data.size_bytes(), alignment, data.data());
    }

  private:
    size_t Allocate(size_t size, size_t alignment, const void* data);
    
    Buffer* buffer_{ nullptr };
    size_t nextAllocOffset_{ 0 };
  };
}