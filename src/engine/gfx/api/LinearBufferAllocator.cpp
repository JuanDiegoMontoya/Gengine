#include "../../PCH.h"
#include "LinearBufferAllocator.h"
#include "Buffer.h"

namespace GFX
{
  std::optional<LinearBufferAllocator> LinearBufferAllocator::Create(Buffer* backingBuffer)
  {
    ASSERT(backingBuffer);
    LinearBufferAllocator alloc;
    alloc.buffer_ = backingBuffer;
    return alloc;
  }

  size_t LinearBufferAllocator::Allocate(size_t size, size_t alignment, const void* data)
  {
    ASSERT(alignment > 0);
    size += (alignment - (size % alignment)) % alignment;
    ASSERT(buffer_->Size() - nextAllocOffset_ > size);

    if (data)
    {
      buffer_->SubData(std::span(reinterpret_cast<const std::byte*>(data), size), nextAllocOffset_);
    }

    return nextAllocOffset_ += size;
  }
}