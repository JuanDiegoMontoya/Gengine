#include "PCH.h"
#include "Material.h"

MaterialManager* MaterialManager::Get()
{
  static MaterialManager manager;
  return &manager;
}

MaterialID MaterialManager::AddMaterial(hashed_string name, const MaterialInfo& materialInfo)
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