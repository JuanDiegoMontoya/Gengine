#pragma once
#include "../Material.h"
#include <optional>

namespace GFX
{
  namespace MaterialManager
  {
    MaterialID AddMaterial(hashed_string name, const MaterialCreateInfo& materialInfo);
    [[nodiscard]] MaterialID GetMaterial(hashed_string name);
    std::optional<MaterialCreateInfo> GetMaterialInfo(MaterialID mat);
  };
}