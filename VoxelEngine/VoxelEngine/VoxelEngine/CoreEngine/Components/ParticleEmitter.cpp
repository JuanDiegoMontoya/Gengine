#include "ParticleEmitter.h"
#include "../ParticleSystem.h"
#include <utility>

Components::ParticleEmitter::ParticleEmitter(ParticleEmitter&& other) noexcept
{
  *this = std::move(other);
}

Components::ParticleEmitter& Components::ParticleEmitter::operator=(ParticleEmitter&& other) noexcept
{
  if (this == &other) return *this;
  this->handle = std::exchange(other.handle, 0);
  this->data = other.data;
  return *this;
}

Components::ParticleEmitter::~ParticleEmitter()
{
  ParticleManager::DestroyParticleEmitter(this->handle);
}