#pragma once
#include <string>
#include <glm/glm.hpp>

namespace GPU
{
  class Texture2D
  {
  public:
    Texture2D(std::string_view path);
    Texture2D(Texture2D&& rhs) noexcept;
    ~Texture2D();

    void Bind(unsigned slot = 0) const;

    glm::ivec2 GetDimensions() const { return dim_; }

  private:
    unsigned rendererID_ = 0;
    glm::ivec2 dim_{};
    static inline const std::string texPath = "./Resources/Textures/";
  };
}