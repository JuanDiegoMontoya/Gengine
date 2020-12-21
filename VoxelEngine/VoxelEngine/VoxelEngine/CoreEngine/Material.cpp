#include "EnginePCH.h"
#include "Material.h"

std::shared_ptr<MaterialHandle> MaterialManager::CreateMaterial(MaterialInfo materialData, entt::hashed_string name)
{
  MaterialInternalInfo info;
  info.tex2Dpaths = materialData.tex2Dpaths;
  info.shaderID = materialData.shaderID;
  for (const auto& path : materialData.tex2Dpaths)
  {
    info.textures.emplace_back(path);
  }
  auto res = std::make_shared<MaterialHandle>(name.value());
  handleMap_.emplace(name.value(), res);
  materials_.emplace(name.value(), std::move(info));
  return std::move(res);
}

std::shared_ptr<MaterialHandle> MaterialManager::GetMaterial(entt::hashed_string name)
{
  return std::shared_ptr<MaterialHandle>(handleMap_[name.value()]);
}

void MaterialManager::DestroyMaterial(MaterialID handle)
{
  materials_.erase(handle);
  handleMap_.erase(handle);
}

//const MaterialInfo& MaterialManager::GetMaterialInfo(Material material)
//{
//  auto fr = materials.find(material);
//  ASSERT(fr != materials.end());
//  return fr->second.data;
//}