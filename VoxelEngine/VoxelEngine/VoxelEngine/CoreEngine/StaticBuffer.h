#pragma once

enum class Target
{
  VBO = GL_ARRAY_BUFFER,
  SSBO = GL_SHADER_STORAGE_BUFFER,
  ABO = GL_ATOMIC_COUNTER_BUFFER,
  DIB = GL_DRAW_INDIRECT_BUFFER,
  ParameterBuffer = GL_PARAMETER_BUFFER,
  UBO = GL_UNIFORM_BUFFER,
};

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
  bool operator==(const StaticBuffer&) const = default;

  // updates a subset of the buffer's data store
  void SubData(const void* data, GLuint size, GLuint offset = 0);

  // for binding everything EXCEPT SSBOs and UBOs
  template<Target T> 
  void Bind() const
  {
    static_assert(T != Target::SSBO && T != Target::UBO, "Cannot bind SSBO or UBO with this method!");
    glBindBuffer((GLenum)T, rendererID_);
  }
 
  // for binding SSBOs and UBOs
  template<Target T>
  void Bind(GLuint index) const
  {
    static_assert(T == Target::SSBO || T == Target::UBO, "Only bind SSBO or UBO with this method!");
    glBindBuffer((GLenum)T, rendererID_);
    glBindBufferBase((GLenum)T, index, rendererID_);
  }

  template<Target T>
  void Unbind() const
  {
    glBindBuffer((GLenum)T, 0);
  }

  // for when this class doesn't offer enough functionality
  // Note: constness is shallow!
  GLuint GetID() const { return rendererID_; }

private:

  GLuint rendererID_ = 0;
};