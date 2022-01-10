#pragma once
#include "../api/Texture.h"
#include "../Camera.h"

namespace GFX::FX::Volumetric
{
  struct CommonParameters
  {
    const Camera& camera;
    float nearPlane{};
    float farPlane{};
  };

  struct AccumulateParameters
  {
    CommonParameters common;
    //const TextureView& sourceVolume;
    const TextureView& densityVolume;
  };

  struct MarchParameters
  {
    CommonParameters common;
    const TextureView& sourceVolume;
    const TextureView& targetVolume;
    TextureSampler& scratchSampler;
  };

  struct ApplyParameters
  {
    CommonParameters common;
    const TextureView& colorTexture;
    const TextureView& depthTexture;
    const TextureView& targetTexture;
    const TextureView& sourceVolume;
    const TextureView& blueNoiseTexture;
    TextureSampler& scratchSampler;
  };

  void CompileShaders();

  void Reset(const TextureView& volume);
  void Accumulate(AccumulateParameters params);
  void March(MarchParameters params);
  void ApplyDeferred(ApplyParameters params);
}