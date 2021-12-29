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
    LinearBufferAllocator(const LinearBufferAllocator&) = delete;
    LinearBufferAllocator(LinearBufferAllocator&&) = default;
    LinearBufferAllocator& operator=(const LinearBufferAllocator&) = delete;
    LinearBufferAllocator& operator=(LinearBufferAllocator&&) = default;

    static std::optional<LinearBufferAllocator> Create(Buffer* backingBuffer);

    // returns a byte offset in the buffer
    template<typename T>
    size_t Allocate(std::span<T> data, size_t alignment)
    {
      return Allocate(data.size_bytes(), alignment, data.data());
    }

  private:
    LinearBufferAllocator() {};
    size_t Allocate(size_t size, size_t alignment, const void* data);
    
    Buffer* buffer_{ nullptr };
    size_t nextAllocOffset_{ 0 };
  };
}