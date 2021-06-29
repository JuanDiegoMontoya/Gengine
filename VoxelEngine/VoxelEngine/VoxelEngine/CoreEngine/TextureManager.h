#pragma once
#include <Utilities/HashedString.h>
#include <optional>

namespace GFX
{
  class Texture;

  class TextureManager
  {
  public:
    static TextureManager* Get();

    Texture* GetTexture(hashed_string name) const;
    bool AddTexture(hashed_string name, Texture&& texture);

  private:
    TextureManager();
    ~TextureManager();
    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;

    struct TextureManagerStorage* storage{};
  };
}