#include "EnginePCH.h"
#include "Material.h"
#include <CoreEngine/GAssert.h>

MaterialHandle MaterialManager::CreateMaterial(MaterialInfo materialData)
{
  MaterialInternalInfo info;
  info.tex2Dpaths = materialData.tex2Dpaths;
  info.shaderID = materialData.shaderID;
  for (const auto& path : materialData.tex2Dpaths)
    info.textures.emplace_back(path);
  auto aa = materials.emplace(nextKey, std::move(info));
  return nextKey++;
}

//const MaterialInfo& MaterialManager::GetMaterialInfo(Material material)
//{
//  auto fr = materials.find(material);
//  ASSERT(fr != materials.end());
//  return fr->second.data;
//}