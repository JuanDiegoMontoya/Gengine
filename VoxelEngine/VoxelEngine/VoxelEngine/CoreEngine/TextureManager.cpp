#include "PCH.h"
#include "TextureManager.h"
#include <unordered_map>
#include "Texture.h"

namespace GFX
{
  struct TextureManagerStorage
  {
    std::unordered_map<uint32_t, Texture> textures;
  };

  TextureManager* TextureManager::Get()
  {
    static TextureManager manager;
    return &manager;
  }

  TextureManager::TextureManager()
  {
    storage = new TextureManagerStorage;
  }

  TextureManager::~TextureManager()
  {
    delete storage;
  }

  Texture* TextureManager::GetTexture(hashed_string name) const
  {
    if (auto it = storage->textures.find(name); it != storage->textures.end())
      return &it->second;
    return nullptr;
  }

  bool TextureManager::AddTexture(hashed_string name, Texture&& texture)
  {
    auto&& [it, success] = storage->textures.try_emplace(name, std::move(texture));
    return success;
  }
}