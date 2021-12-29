#pragma once
#include <vector>
#include <unordered_map>
#include <functional>
#include <optional>
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

  namespace MaterialManager
  {
    MaterialID AddMaterial(hashed_string name, const MaterialCreateInfo& materialInfo);
    [[nodiscard]] MaterialID GetMaterial(hashed_string name);
    std::optional<MaterialCreateInfo> GetMaterialInfo(MaterialID mat);
  };
}