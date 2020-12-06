#pragma once
#include <CoreEngine/vbo_layout.h>

namespace GPU
{
  class VAO
  {
  public:
    VAO();
    ~VAO();

    void AddBuffer(const StaticBuffer& vbo, const VBOlayout& layout);

    void Bind() const;
    void Unbind() const;

    GLuint GetID() const { return rendererID_; }

  private:
    GLuint rendererID_;
  };
}