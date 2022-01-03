#pragma once
#include "../api/Texture.h"
#include "../Camera.h"

namespace GFX::FX::Volumetric
{
  struct AccumulateParameters
  {
    //const TextureView& sourceVolume;
    const TextureView& densityVolume;
    const Camera& camera;
    float nearPlane{};
    float farPlane{};
  };

  struct MarchParameters
  {
    const TextureView& sourceVolume;
    const TextureView& targetVolume;
    TextureSampler& scratchSampler;
    const Camera& camera;
    float nearPlane{};
    float farPlane{};
  };

  struct ApplyParameters
  {
    const TextureView& colorTexture;
    const TextureView& depthTexture;
    const TextureView& targetTexture;
    const TextureView& sourceVolume;
    const TextureView& blueNoiseTexture;
    TextureSampler& scratchSampler;
    const Camera& camera;
    float nearPlane{};
    float farPlane{};
  };

  void CompileShaders();

  void Reset(const TextureView& volume);
  void Accumulate(AccumulateParameters params);
  void March(MarchParameters params);
  void ApplyDeferred(ApplyParameters params);
}