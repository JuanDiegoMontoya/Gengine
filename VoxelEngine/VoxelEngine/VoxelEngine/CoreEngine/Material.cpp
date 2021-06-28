#include "PCH.h"
#include "Material.h"

MaterialID MaterialManager::CreateMaterial(MaterialInfo materialData, hashed_string name)
{
  MaterialInternalInfo info;
  info.tex2Dpaths = materialData.tex2Dpaths;
  info.shaderID = materialData.shaderID;
  for (const auto& path : materialData.tex2Dpaths)
  {
    info.textures.emplace_back(path);
  }

  handleMap_.emplace(name.value(), name);
  materials_.emplace(name.value(), std::move(info));
  return name;
}

MaterialID MaterialManager::GetMaterial(hashed_string name)
{
  return handleMap_[name.value()];
}

//void MaterialManager::DestroyMaterial(MaterialID handle)
//{
//  materials_.erase(handle);
//  handleMap_.erase(handle);
//}

//const MaterialInfo& MaterialManager::GetMaterialInfo(Material material)
//{
//  auto fr = materials.find(material);
//  ASSERT(fr != materials.end());
//  return fr->second.data;
//}