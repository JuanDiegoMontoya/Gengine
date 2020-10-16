#pragma once
#include <string>
#include <glm/glm.hpp>

class Texture2D
{
public:
  Texture2D(std::string_view path);
  Texture2D(Texture2D&& rhs);
  ~Texture2D();

  void Bind(unsigned slot = 0) const;

  glm::ivec2 GetDimensions() const { return dim_; }

private:
  unsigned rendererID_ = 0;
  glm::ivec2 dim_{};
  static inline const std::string texPath = "./Resources/Textures/";
};