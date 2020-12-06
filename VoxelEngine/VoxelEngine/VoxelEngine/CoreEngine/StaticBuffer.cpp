#include <CoreEngine/GraphicsIncludes.h>
#include <CoreEngine/StaticBuffer.h>


namespace GPU
{
  StaticBuffer::StaticBuffer(const void* data, GLuint size, GLbitfield glflags)
  {
    glCreateBuffers(1, &rendererID_);
    glNamedBufferStorage(rendererID_, std::max(size, 1u), data, glflags);
  }

  StaticBuffer::StaticBuffer(const StaticBuffer& other)
  {
    glCreateBuffers(1, &rendererID_);
    GLint size{};
    glGetNamedBufferParameteriv(other.rendererID_, GL_BUFFER_SIZE, &size);
    glCopyNamedBufferSubData(other.rendererID_, rendererID_, 0, 0, size);
  }

  StaticBuffer::StaticBuffer(StaticBuffer&& other) noexcept
  {
    rendererID_ = std::exchange(other.rendererID_, 0);
  }

  StaticBuffer::~StaticBuffer()
  {
    glDeleteBuffers(1, &rendererID_);
  }

  void StaticBuffer::SubData(const void* data, GLuint size, GLuint offset)
  {
    glNamedBufferSubData(rendererID_, offset, size, data);
  }
}