#include "../../PCH.h"
#include "MaterialManager.h"

namespace GFX
{
  static std::unordered_map<MaterialID, MaterialCreateInfo> materials_;

  MaterialID MaterialManager::AddMaterial(hashed_string name, const MaterialCreateInfo& materialInfo)
  {
    ASSERT_MSG(materialInfo.viewSamplers.size() <= 16, "Per-material view/sampler limit exceeded");
    if (materials_.find(name) != materials_.end())
      return 0;
    materials_.emplace(name, materialInfo); // invoke copy constructors
    return name;
  }

  MaterialID MaterialManager::GetMaterial(hashed_string name)
  {
    // hide mapping from the user in case it changes
    return name;
  }

  std::optional<MaterialCreateInfo> MaterialManager::GetMaterialInfo(MaterialID mat)
  {
    if (auto it = materials_.find(mat); it != materials_.end())
      return it->second;
    return std::nullopt;
  }
}