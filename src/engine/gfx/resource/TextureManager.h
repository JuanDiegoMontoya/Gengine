#pragma once
#include <utility/HashedString.h>

namespace GFX
{
  class Texture;

  namespace TextureManager
  {
    bool AddTexture(hashed_string name, Texture&& texture);
    [[nodiscard]] Texture* GetTexture(hashed_string name);
  };
}