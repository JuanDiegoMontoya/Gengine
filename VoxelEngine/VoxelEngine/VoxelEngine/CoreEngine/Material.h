#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <Utilities/HashedString.h>
#include <CoreEngine/Texture2D.h>
#include "Texture.h"

class Shader;

using MaterialID = uint32_t;

// the user can use this struct to query and set material data
// idk what else should be here tbh
struct MaterialInfo
{
  std::vector<std::pair<GFX::TextureView&, GFX::TextureSampler&>> viewSamplers;
  hashed_string shaderID;
  // TODO: list of user-set uniforms
};

class MaterialManager
{
public:
  static MaterialManager* Get();
  MaterialID AddMaterial(hashed_string name, const MaterialInfo& materialInfo);
  MaterialID GetMaterial(hashed_string name);

  // TODO: add ways to query material info here
  //static const MaterialInfo& GetMaterialInfo(Material material);

private:
  friend class GraphicsSystem;
  friend class Renderer;

  // There may be an argument to make this mapping public and switch to hashed strings
  // In the meantime, we'll see how this works
  std::unordered_map<uint32_t, MaterialInfo> materials_;

  // 0 is reserved for invalid materials
  //static inline MaterialHandle nextKey = 1;
};