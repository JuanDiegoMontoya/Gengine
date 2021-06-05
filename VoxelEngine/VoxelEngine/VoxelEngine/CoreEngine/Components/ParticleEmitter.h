#pragma once
#include <cinttypes>
#include <glm/glm.hpp>
#include "../MeshUtils.h"

namespace Components
{
  struct ParticleEmitter
  {
    ParticleEmitter() {};
    ParticleEmitter(const ParticleEmitter&) = delete;
    ParticleEmitter& operator=(const ParticleEmitter&) = delete;
    ParticleEmitter(ParticleEmitter&& other) noexcept;
    ParticleEmitter& operator=(ParticleEmitter&& other) noexcept;
    ~ParticleEmitter();

    uint64_t renderFlag = (uint64_t)RenderFlags::Default;
    uint64_t handle{};

    struct
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