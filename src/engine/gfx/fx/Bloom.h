#pragma once
#include "../api/Texture.h"

namespace GFX::FX
{
  void CompileBloomShaders();
  void ApplyBloom(const TextureView& target, uint32_t passes, float strength, float width,
    const TextureView& scratchTexture, TextureSampler& scratchSampler);
}