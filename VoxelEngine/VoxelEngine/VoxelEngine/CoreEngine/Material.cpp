#include "Material.h"
#include <GAssert.h>
#include <Texture2D.h>

Material MaterialManager::CreateMaterial(MaterialInfo materialData)
{
  MaterialInternalInfo info;
  info.data = materialData;
  info.texture = std::make_unique<Texture2D>(materialData.file);
  auto aa = materials.emplace(nextKey, std::move(info));
  return nextKey++;
}

const MaterialInfo& MaterialManager::GetMaterialInfo(Material material)
{
  auto fr = materials.find(material);
  ASSERT(fr != materials.end());
  return fr->second.data;
}
