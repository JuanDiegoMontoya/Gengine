#include "Material.h"
#include <GAssert.h>
#include <Texture2D.h>

MaterialHandle MaterialManager::CreateMaterial(MaterialInfo materialData)
{
  MaterialInternalInfo info;
  info.data = materialData;
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
