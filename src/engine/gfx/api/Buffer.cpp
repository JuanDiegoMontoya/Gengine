#include "../../PCH.h"
#include <glad/glad.h>
#include "Buffer.h"

uint32_t getSetBit(uint32_t val, uint32_t bit)
{
  return val & (1 << bit) ? bit : 0;
}

namespace GFX
{
  namespace
  {
    GLenum targets[]
    {
      GL_ARRAY_BUFFER,
      GL_SHADER_STORAGE_BUFFER,
      GL_ATOMIC_COUNTER_BUFFER,
      GL_DRAW_INDIRECT_BUFFER,
      GL_PARAMETER_BUFFER,
      GL_UNIFORM_BUFFER,
    };

    GLbitfield bufferFlags[]
    {
      0,
      GL_DYNAMIC_STORAGE_BIT,
      GL_CLIENT_STORAGE_BIT,
      GL_MAP_READ_BIT,
      GL_MAP_WRITE_BIT,
      GL_MAP_PERSISTENT_BIT,
      GL_MAP_COHERENT_BIT,
    };
  }


  Buffer::Buffer(const void* data, size_t size, BufferFlags flags)
    : size_(std::max(size, 1ull))
  {
    GLbitfield glflags{};
    for (int i = 1; i < _countof(bufferFlags); i++)
      glflags |= bufferFlags[getSetBit((uint32_t)flags, i)];
    glCreateBuffers(1, &rendererID_);
    glNamedBufferStorage(rendererID_, size_, data, glflags);
  }

  Buffer::Buffer(Buffer&& old) noexcept
  {
    *this = std::move(old);
  }

  Buffer& Buffer::operator=(Buffer&& old) noexcept
  {
    if (&old == this) return *this;
    rendererID_ = std::exchange(old.rendererID_, 0);
    size_ = std::exchange(old.size_, 0);
    return *this;
  }

  Buffer::~Buffer()
  {
    ASSERT_MSG(!IsMapped(), "Buffer should not be mapped at time of destruction.");
    glDeleteBuffers(1, &rendererID_);
  }

  void Buffer::SubData(const void* data, size_t size, size_t offset)
  {
    glNamedBufferSubData(rendererID_, static_cast<GLuint>(offset), static_cast<GLuint>(size), data);
  }

  void* Buffer::Map()
  {
    return glMapNamedBuffer(rendererID_, GL_READ_WRITE);
  }

  void Buffer::Unmap()
  {
    ASSERT_MSG(IsMapped(), "The buffer is not mapped.");
    glUnmapNamedBuffer(rendererID_);
  }

  bool Buffer::IsMapped()
  {
    if (!rendererID_) return false;
    GLint mapped{ GL_FALSE };
    glGetNamedBufferParameteriv(rendererID_, GL_BUFFER_MAPPED, &mapped);
    return mapped;
  }

  void Buffer::BindBuffer(uint32_t target)
  {
    glBindBuffer(targets[target], rendererID_);
  }

  void Buffer::BindBufferBase(uint32_t target, uint32_t slot)
  {
    glBindBufferBase(targets[target], slot, rendererID_);
  }
}