#pragma once
#include "../api/Texture.h"
#include "../Camera.h"

namespace GFX::FX
{
  struct FogParameters
  {
    const TextureView& sourceColor;
    const TextureView& sourceDepth;
    const TextureView& targetColor;
    TextureSampler scratchSampler;
    const Camera& camera;
    float a{};
    float b{};
    float heightOffset{};
    float fog2Density{};
    glm::vec3 albedo{};
    float beer{};
    float powder{};
  };

  void CompileFogShader();

  void ApplyFog(FogParameters params);
}