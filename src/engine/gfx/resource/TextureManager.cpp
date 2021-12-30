#include "../../PCH.h"
#include "TextureManager.h"
#include <unordered_map>
#include "../api/Texture.h"

namespace GFX
{
  namespace TextureManager
  {
    namespace
    {
      static std::unordered_map<uint32_t, Texture> textures_;
    }

    Texture* GetTexture(hashed_string name)
    {
      if (auto it = textures_.find(name); it != textures_.end())
        return &it->second;
      return nullptr;
    }

    bool AddTexture(hashed_string name, Texture&& texture)
    {
      auto&& [it, success] = textures_.try_emplace(name, std::move(texture));
      return success;
    }
  }
}