#pragma once
#include <span>
#include <string>
#include <glm/glm.hpp>

namespace GPU
{
  class TextureArray
  {
  public:
    TextureArray(std::span<std::string> textures, glm::ivec2 xyDim);
    ~TextureArray();

    void Bind(unsigned slot = 0) const;

    glm::ivec2 GetDimensions() const { return dim; }

  private:
    unsigned rendererID_ = 0;
    glm::ivec2 dim{};
    static inline const std::string texPath = "./Resources/Textures/";
  };
}