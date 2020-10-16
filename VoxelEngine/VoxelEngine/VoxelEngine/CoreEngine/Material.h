#pragma once
#include <string>
#include <unordered_map>
#include <memory>

class Texture2D;

using Material = uint32_t;

// the user can use this struct to query and set material data
// idk what else should be here tbh
struct MaterialInfo
{
  std::string file;
};

class MaterialManager
{
public:
  static Material CreateMaterial(MaterialInfo materialData);
  static const MaterialInfo& GetMaterialInfo(Material material);

private:
  friend class Renderer;

  // the user doesn't get to know about this
  struct MaterialInternalInfo
  {
    MaterialInfo data;
    std::unique_ptr<Texture2D> texture = nullptr;
  };

  static inline std::unordered_map<Material, MaterialInternalInfo> materials;

  // 0 is reserved for invalid materials
  static inline Material nextKey = 1;
};