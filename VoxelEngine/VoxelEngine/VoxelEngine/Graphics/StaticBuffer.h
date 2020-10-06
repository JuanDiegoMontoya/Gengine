#pragma once

// general-purpose immutable buffer storage
class StaticBuffer
{
public:
  StaticBuffer(const void* data, GLuint size, GLbitfield glflags = GL_DYNAMIC_STORAGE_BIT);

  // copies another buffer's data store and contents
  StaticBuffer(const StaticBuffer& other);
  StaticBuffer(StaticBuffer&& other) noexcept;
  ~StaticBuffer();

  StaticBuffer& operator=(const StaticBuffer&) = delete;
  StaticBuffer& operator=(StaticBuffer&&) = delete;

  // updates a subset of the buffer's data store
  void SubData(const void* data, GLuint size, GLuint offset = 0);

  template<GLuint Target> 
  void Bind(GLuint index) const
  {
    glBindBuffer(Target, rendererID_);
  }

  template<GLuint Target>
  void Unbind() const
  {
    glBindBuffer(Target, 0);
  }

  // for when this class doesn't offer enough functionality
  // not const because the ID can be used to modify the buffer
  GLuint GetID() { return rendererID_; }

private:
  GLuint rendererID_ = 0;
};