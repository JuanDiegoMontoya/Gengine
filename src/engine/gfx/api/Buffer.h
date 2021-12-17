#pragma once
#include <utility/Flags.h>

namespace GFX
{
  enum class Target
  {
    VERTEX_BUFFER,
    SHADER_STORAGE_BUFFER,
    ATOMIC_BUFFER,
    DRAW_INDIRECT_BUFFER,
    PARAMETER_BUFFER,
    UNIFORM_BUFFER,
  };

  enum class BufferFlag : uint32_t
  {
    NONE = 1 << 0,
    DYNAMIC_STORAGE = 1 << 1,
    CLIENT_STORAGE = 1 << 2,

    MAP_READ = 1 << 3,
    MAP_WRITE = 1 << 4,
    MAP_PERSISTENT = 1 << 5,
    MAP_COHERENT = 1 << 6,
  };
  DECLARE_FLAG_TYPE(BufferFlags, BufferFlag, uint32_t)


  // General-purpose immutable buffer storage
  class Buffer
  {
  public:
    Buffer(const void* data, size_t size, BufferFlags flags = BufferFlag::DYNAMIC_STORAGE);

    // copies another buffer's data store and contents
    Buffer(const Buffer& other) = delete;
    Buffer(Buffer&& other) noexcept;
    Buffer& operator=(Buffer&& other) noexcept;
    ~Buffer();

    Buffer& operator=(const Buffer&) = delete;
    bool operator==(const Buffer&) const = default;

    // updates a subset of the buffer's data store
    void SubData(const void* data, size_t size, size_t offset = 0);

    // for binding everything EXCEPT SSBOs and UBOs
    template<Target T>
    void Bind()
    {
      static_assert(T != Target::SHADER_STORAGE_BUFFER && T != Target::UNIFORM_BUFFER, "SSBO and UBO targets require an index.");
      BindBuffer((uint32_t)T);
    }

    // for binding SSBOs and UBOs
    template<Target T>
    void Bind(uint32_t index)
    {
      static_assert(T == Target::SHADER_STORAGE_BUFFER || T == Target::UNIFORM_BUFFER, "Only SSBO and UBO targets use an index.");
      BindBuffer((uint32_t)T);
      BindBufferBase((uint32_t)T, index);
    }

    // Gets a read+write pointer back
    [[nodiscard]] void* Map();

    void Unmap();

    // TODO: this function doesn't work
    bool IsMapped();

    // for when this class doesn't offer enough functionality itself
    auto GetID() const { return rendererID_; }

  private:
    void BindBuffer(uint32_t target);
    void BindBufferBase(uint32_t target, uint32_t slot);

    uint32_t rendererID_{};
    uint32_t size_{};
  };
}