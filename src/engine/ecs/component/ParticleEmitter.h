#pragma once
#include <cstdint>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

namespace Component
{
  struct ParticleEmitter
  {
    uint64_t handle{};

    struct ParticleEmitterParams
    {
      glm::vec3 minParticleOffset{ -1 };
      glm::vec3 maxParticleOffset{ 1 };
      glm::vec3 minParticleVelocity{ -1 };
      glm::vec3 maxParticleVelocity{ 1 };
      glm::vec3 minParticleAccel{ -1 };
      glm::vec3 maxParticleAccel{ 1 };
      glm::vec2 minParticleScale{ 1 };
      glm::vec2 maxParticleScale{ 1 };
      glm::vec4 minParticleColor{ 1 };
      glm::vec4 maxParticleColor{ 1 };

      float interval{ 0.1f };
      float minLife{ 1 };
      float maxLife{ 1 };
    } data;
  };
}