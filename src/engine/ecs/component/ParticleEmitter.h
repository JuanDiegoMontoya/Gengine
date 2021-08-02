#pragma once
#include <cinttypes>
#include <glm/vec3.hpp>
#include <engine/gfx/MeshUtils.h>

namespace Component
{
  struct ParticleEmitter
  {
    uint64_t renderFlag = (uint64_t)RenderFlags::Default;
    uint64_t handle{};

    struct ParticleEmitterData
    {
      glm::vec3 minParticleOffset{ -1 };
      glm::vec3 maxParticleOffset{ 1 };
      glm::vec3 minParticleVelocity{ -1 };
      glm::vec3 maxParticleVelocity{ 1 };
      glm::vec3 minParticleAccel{ -1 };
      glm::vec3 maxParticleAccel{ 1 };
      glm::vec2 minParticleScale{ .1f };
      glm::vec2 maxParticleScale{ 1 };
      glm::vec4 minParticleColor{ 1 };
      glm::vec4 maxParticleColor{ 1 };

      float interval{ 0.1f };
      float minLife{ 1 };
      float maxLife{ 1 };
    } data;
  };
}