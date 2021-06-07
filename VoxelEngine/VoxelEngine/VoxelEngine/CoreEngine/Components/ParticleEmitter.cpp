#include "ParticleEmitter.h"
#include "../ParticleSystem.h"
#include <utility>

Component::ParticleEmitter::ParticleEmitter(ParticleEmitter&& other) noexcept
{
  *this = std::move(other);
}

Component::ParticleEmitter& Component::ParticleEmitter::operator=(ParticleEmitter&& other) noexcept
{
  if (this == &other) return *this;
  this->handle = std::exchange(other.handle, 0);
  this->data = other.data;
  return *this;
}

Component::ParticleEmitter::~ParticleEmitter()
{
}