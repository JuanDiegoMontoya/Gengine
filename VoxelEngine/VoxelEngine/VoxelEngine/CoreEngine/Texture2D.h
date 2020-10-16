#pragma once
#include <string>

class Texture2D
{
public:
  Texture2D(std::string_view path);
  ~Texture2D();

  void Bind(GLuint slot = 0) const;

  glm::ivec2 GetDimensions() { return dim_; }

private:
  GLuint rendererID_ = 0;
  glm::ivec2 dim_{};
  static inline const std::string texPath = "./Resources/Textures/";
};