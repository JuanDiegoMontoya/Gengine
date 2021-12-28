#pragma once
#include <vector>
#include <unordered_map>
#include <functional>
#include <utility/HashedString.h>
#include "api/Texture.h"
#include "api/Shader.h"

namespace GFX
{
  using MaterialID = uint32_t;

  struct PerMaterialUniformData
  {
    hashed_string id;
    std::function<void(hashed_string id, Shader& shader)> Setter;
  };

  // the user can use this struct to query and set material data
  // idk what else should be here tbh
  struct MaterialCreateInfo
  {
    hashed_string shaderID;
    std::vector<std::pair<GFX::TextureView, GFX::TextureSampler>> viewSamplers;
    std::vector<PerMaterialUniformData> materialUniforms;
  };

  class MaterialManager
  {
  public:
    [[nodiscard]] static MaterialManager* Get();
    MaterialID AddMaterial(hashed_string name, const MaterialCreateInfo& materialInfo);
    [[nodiscard]] MaterialID GetMaterial(hashed_string name);

    // TODO: add ways to query material info here
    //static const MaterialInfo& GetMaterialInfo(Material material);
    auto GetMaterialInfo(MaterialID mat) const { return materials_.find(mat); }

  private:
    friend class GraphicsSystem;

    // There may be an argument to make this mapping public and switch to hashed strings
    // In the meantime, we'll see how this works
    std::unordered_map<MaterialID, MaterialCreateInfo> materials_;

    // 0 is reserved for invalid materials
    //static inline MaterialHandle nextKey = 1;
  };
}