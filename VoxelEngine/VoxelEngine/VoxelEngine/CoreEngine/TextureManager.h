#pragma once
#include <Utilities/HashedString.h>
#include <optional>

namespace GFX
{
  class Texture;

  class TextureManager
  {
  public:
    [[nodiscard]] static TextureManager* Get();
    bool AddTexture(hashed_string name, Texture&& texture);
    [[nodiscard]] Texture* GetTexture(hashed_string name) const;

  private:
    TextureManager();
    ~TextureManager();
    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;

    struct TextureManagerStorage* storage{};
  };
}