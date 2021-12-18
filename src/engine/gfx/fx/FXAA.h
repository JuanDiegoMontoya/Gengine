#pragma once
#include "../api/Texture.h"

namespace GFX
{
  namespace FX
  {
    void ApplyFXAA(const TextureView& source, const TextureView& target,
      float contrastThreshold, float relativeThreshold, 
      float pixelBlendStrength, float edgeBlendStrength,
      TextureSampler& scratchSampler);
  }
}