#pragma once
#include <CoreEngine/vbo_layout.h>

class VAO
{
public:
  VAO();
  ~VAO();

  void AddBuffer(const StaticBuffer& vbo, const VBOlayout& layout);

  void Bind() const;
  void Unbind() const;

private:
  GLuint rendererID_;
};