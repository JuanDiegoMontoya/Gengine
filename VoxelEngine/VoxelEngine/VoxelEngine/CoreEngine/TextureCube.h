#pragma once
#include <string>
#include <glm/glm.hpp>

namespace GFX
{
  class TextureCube
  {
  public:
    TextureCube(std::span<std::string, 6> paths);
    TextureCube(const TextureCube& rhs) = delete;
    TextureCube& operator=(TextureCube&& rhs) noexcept;
    TextureCube(TextureCube&& rhs) noexcept;
    ~TextureCube();

    void Bind(unsigned slot = 0) const;

  private:
    unsigned rendererID_ = 0;
    static inline const std::string texPath = "./Resources/Textures/";
  };
}