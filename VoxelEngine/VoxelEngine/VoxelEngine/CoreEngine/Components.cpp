#include "EnginePCH.h"
#include "Components.h"
#include "StaticBuffer.h"

// TODO: temp and bad
#include "Texture2D.h"

Components::ParticleEmitter::ParticleEmitter(uint32_t maxp, std::string tex)
  : maxParticles(maxp)
{
  struct Particle // std430 layout
  {
    glm::vec4 pos{ -1 };
    glm::vec4 velocity{ 1 };
    glm::vec4 accel{ 1 };
    glm::vec4 color{ 0 };
    float life{ 0 };
    int alive{ 0 };
    glm::vec2 scale{ 1 };
  };

  auto tp = std::make_unique<Particle[]>(maxParticles);
  particleBuffer = std::make_unique<GFX::StaticBuffer>(tp.get(), sizeof(Particle) * maxParticles,
    GFX::BufferFlag::DYNAMIC_STORAGE);

  const size_t bytes = sizeof(int32_t) + maxParticles * sizeof(int32_t);
  uint8_t* mem = new uint8_t[bytes];
  reinterpret_cast<int32_t&>(mem[0]) = maxParticles;
  int32_t val = 0;
  for (size_t i = sizeof(int32_t); i < bytes; i += sizeof(int32_t))
  {
    reinterpret_cast<int32_t&>(mem[i]) = val++;
  }
  freeStackBuffer = std::make_unique<GFX::StaticBuffer>(mem, bytes,
    GFX::BufferFlag::DYNAMIC_STORAGE | GFX::BufferFlag::CLIENT_STORAGE);
  delete[] mem;

  texture = std::make_unique<GFX::Texture2D>(tex);
}

Components::ParticleEmitter::ParticleEmitter(ParticleEmitter&& rhs) noexcept
  : maxParticles(rhs.maxParticles)
{
  minParticleOffset = std::move(rhs.minParticleOffset);
  maxParticleOffset = std::move(rhs.maxParticleOffset);
  minParticleVelocity = std::move(rhs.minParticleVelocity);
  maxParticleVelocity = std::move(rhs.maxParticleVelocity);
  minParticleAccel = std::move(rhs.minParticleAccel);
  maxParticleAccel = std::move(rhs.maxParticleAccel);
  minParticleScale = std::move(rhs.minParticleScale);
  maxParticleScale = std::move(rhs.maxParticleScale);
  minParticleColor = std::move(rhs.minParticleColor);
  maxParticleColor = std::move(rhs.maxParticleColor);
  timer = std::move(rhs.timer);
  interval = std::move(rhs.interval);
  minLife = std::move(rhs.minLife);
  maxLife = std::move(rhs.maxLife);
  numParticles = std::move(rhs.numParticles);
  renderFlag = std::move(rhs.renderFlag);
  particleBuffer = std::move(rhs.particleBuffer);
  freeStackBuffer = std::move(rhs.freeStackBuffer);
  texture = std::move(rhs.texture);
}

Components::ParticleEmitter& Components::ParticleEmitter::operator=(ParticleEmitter&& rhs) noexcept
{
  if (&rhs == this) return *this;
  return *new (this) ParticleEmitter(std::move(rhs));
}
