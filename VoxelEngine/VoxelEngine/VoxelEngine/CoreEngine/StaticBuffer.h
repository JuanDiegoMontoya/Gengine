#pragma once
#include <type_traits>

namespace GPU
{
  enum class Target
  {
    VBO = GL_ARRAY_BUFFER,
    SSBO = GL_SHADER_STORAGE_BUFFER,
    ATOMIC_BUFFER = GL_ATOMIC_COUNTER_BUFFER,
    DIB = GL_DRAW_INDIRECT_BUFFER,
    PARAMETER_BUFFER = GL_PARAMETER_BUFFER,
    UBO = GL_UNIFORM_BUFFER,
  };

  enum class BufferFlags : uint32_t
  {
    NONE = 0,
    DYNAMIC_STORAGE = GL_DYNAMIC_STORAGE_BIT,
    CLIENT_STORAGE = GL_CLIENT_STORAGE_BIT,

    MAP_READ = GL_MAP_READ_BIT,
    MAP_WRITE = GL_MAP_WRITE_BIT,
    MAP_PERSISTENT = GL_MAP_PERSISTENT_BIT,
    MAP_COHERENT = GL_MAP_COHERENT_BIT,
  };
  inline BufferFlags operator|(BufferFlags a, BufferFlags b)
  {
    using T = std::underlying_type_t<BufferFlags>;
    return static_cast<BufferFlags>(static_cast<T>(a) | static_cast<T>(b));
  }
  inline BufferFlags operator&(BufferFlags a, BufferFlags b)
  {
    using T = std::underlying_type_t<BufferFlags>;
    return static_cast<BufferFlags>(static_cast<T>(a) & static_cast<T>(b));
  }


  // General-purpose immutable buffer storage
  class StaticBuffer
  {
  public:
    StaticBuffer(const void* data, GLuint size, BufferFlags flags = BufferFlags::DYNAMIC_STORAGE);

    // copies another buffer's data store and contents
    StaticBuffer(const StaticBuffer& other);
    StaticBuffer(StaticBuffer&& other) noexcept;
    ~StaticBuffer();

    StaticBuffer& operator=(const StaticBuffer&) = delete;
    StaticBuffer& operator=(StaticBuffer&&) = delete;
    bool operator==(const StaticBuffer&) const = default;

    // updates a subset of the buffer's data store
    void SubData(const void* data, GLuint size, GLuint offset = 0);

    // for binding everything EXCEPT SSBOs and UBOs
    template<Target T>
    void Bind()
    {
      static_assert(T != Target::SSBO && T != Target::UBO, "SSBO and UBO targets require an index.");
      glBindBuffer((GLenum)T, rendererID_);
    }

    // for binding SSBOs and UBOs
    template<Target T>
    void Bind(GLuint index)
    {
      static_assert(T == Target::SSBO || T == Target::UBO, "Only SSBO and UBO targets use an index.");
      glBindBuffer((GLenum)T, rendererID_);
      glBindBufferBase((GLenum)T, index, rendererID_);
    }

    // TODO: evaluate whether or not this is actually useful
    template<Target T>
    void Unbind() const
    {
      glBindBuffer((GLenum)T, 0);
    }

    // Gets a read+write pointer back
    void* GetMappedPointer()
    {
      return glMapNamedBuffer(rendererID_, GL_READ_WRITE);
    }

    void UnmapPointer()
    {
      ASSERT_MSG(IsMapped(), "The buffer is not mapped.");
      glUnmapNamedBuffer(rendererID_);
    }

    // TODO: this function doesn't work
    bool IsMapped()
    {
      GLint mapped{ GL_FALSE };
      glGetNamedBufferParameteriv(rendererID_, GL_BUFFER_MAPPED, &mapped);
      return mapped;
    }

    // for when this class doesn't offer enough functionality itself
    GLuint GetID() { return rendererID_; }

  private:
    GLuint rendererID_{ 0 };
  };
}