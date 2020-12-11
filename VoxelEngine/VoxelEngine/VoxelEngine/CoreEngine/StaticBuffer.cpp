#include "EnginePCH.h"
#include <CoreEngine/GraphicsIncludes.h>
#include <CoreEngine/StaticBuffer.h>

namespace GPU
{
  StaticBuffer::StaticBuffer(const void* data, GLuint size, BufferFlags flags)
  {
    glCreateBuffers(1, &rendererID_);
    glNamedBufferStorage(rendererID_, std::max(size, 1u), data, static_cast<GLbitfield>(flags));
  }

  StaticBuffer::StaticBuffer(const StaticBuffer& other)
  {
    glCreateBuffers(1, &rendererID_);
    GLint size{};
    glGetNamedBufferParameteriv(other.rendererID_, GL_BUFFER_SIZE, &size);
    GLbitfield flags{};
    glGetNamedBufferParameteriv(other.rendererID_, GL_BUFFER_STORAGE_FLAGS, (GLint*)&flags);
    glNamedBufferStorage(rendererID_, size, nullptr, flags);
    glCopyNamedBufferSubData(other.rendererID_, rendererID_, 0, 0, size);
  }

  StaticBuffer::StaticBuffer(StaticBuffer&& other) noexcept
  {
    rendererID_ = std::exchange(other.rendererID_, 0);
  }

  StaticBuffer::~StaticBuffer()
  {
    ASSERT_MSG(!IsMapped(), "Buffer should not be mapped at time of destruction.");
    glDeleteBuffers(1, &rendererID_);
  }

  void StaticBuffer::SubData(const void* data, GLuint size, GLuint offset)
  {
    glNamedBufferSubData(rendererID_, offset, size, data);
  }
}