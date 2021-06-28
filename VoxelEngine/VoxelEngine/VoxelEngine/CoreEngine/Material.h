#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <Utilities/HashedString.h>
#include <CoreEngine/Texture2D.h>

class Shader;

using MaterialID = uint32_t;

// the user can use this struct to query and set material data
// idk what else should be here tbh
struct MaterialInfo
{
  std::vector<std::string> tex2Dpaths;
  hashed_string shaderID;
  // user-exposed uniforms here (user puts a list of uniform names and variants)
};

class MaterialManager
{
public:
  static MaterialID CreateMaterial(MaterialInfo materialData, hashed_string name);
  static MaterialID GetMaterial(hashed_string name);

  // TODO: add ways to query material info here
  //static const MaterialInfo& GetMaterialInfo(Material material);

private:
  friend class GraphicsSystem;
  friend class Renderer;

  // the user doesn't get to know about this
  struct MaterialInternalInfo
  {
    std::vector<std::string> tex2Dpaths;
    hashed_string shaderID;
    std::vector<GFX::Texture2D> textures;
  };

  // There may be an argument to make this mapping public and switch to hashed strings
  // In the meantime, we'll see how this works
  static inline std::unordered_map<uint32_t, MaterialInternalInfo> materials_;
  static inline std::unordered_map<uint32_t, MaterialID> handleMap_;

  // 0 is reserved for invalid materials
  //static inline MaterialHandle nextKey = 1;
};