#pragma once
#include "../api/Texture.h"
#include "../Camera.h"
#include <array>

namespace GFX::FX
{
  // create with designated initialization
  struct ReflectionsCommonParameters
  {
    const TextureView& gbDepth;
    const TextureView& gbColor;
    const TextureView& gbNormal;
    const TextureView& gbPBR;
    TextureSampler& scratchSampler;
    TextureSampler& scratchSampler2;
    const Camera& camera;
  };

  struct TraceCubemapReflectionsParameters
  {
    ReflectionsCommonParameters common;
    const TextureView& target;
    const std::array<Camera, 6>& cameras;
    const TextureView& probeColor;
    const TextureView& probeDistance;
    const std::array<std::optional<TextureView>, 6>& depthViews;
    const std::array<std::optional<TextureView>, 6>& distanceViews;
    const TextureView& skybox;
    const TextureView& blueNoise;
  };

  struct SampleCubemapReflectionsParameters
  {
    ReflectionsCommonParameters common;
    const TextureView& target;
    const TextureView& env;
    const TextureView& blueNoise;
  };

  struct DenoiseReflectionsParameters
  {
    ReflectionsCommonParameters common;
    const TextureView& target;
    const TextureView& scratchTexture;
    struct AtrousParams_t
    {
      uint32_t passes{};
      float nPhi{};
      float pPhi{};
      float stepWidth{};
      std::array<float, 5> kernel;
      std::array<float, 5> offsets;
    }atrousParams;
  };

  struct CompositeReflectionsParameters
  {
    ReflectionsCommonParameters common;
    const TextureView& source; // specular irradiance texture
    const TextureView& target;
  };

  void CompileReflectionShaders();

  void SampleCubemapReflections(SampleCubemapReflectionsParameters params);
  void TraceCubemapReflections(TraceCubemapReflectionsParameters params);
  void DenoiseReflections(DenoiseReflectionsParameters params);
  void CompositeReflections(CompositeReflectionsParameters params);
}