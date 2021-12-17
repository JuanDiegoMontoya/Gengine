#pragma once
#include <engine/gfx/api/Texture.h>
#include <utility/HashedString.h>
#include "../../Timestep.h"

class Scene;
struct ParticleManagerData;

namespace GFX
{
  class Buffer;
  class Texture2D;
  class Renderer;
}

class ParticleManager
{
public:
  static ParticleManager& Get();
  [[nodiscard]] uint64_t MakeParticleEmitter(uint32_t maxp, hashed_string textureName);
  [[nodiscard]] uint64_t MakeParticleEmitter(uint32_t maxp, const GFX::TextureView& texView, const GFX::TextureSampler& sampler);
  void DestroyParticleEmitter(uint64_t handle);

private:
  friend class ParticleSystem;
  friend class GFX::Renderer;

  ParticleManager();
  void BindEmitter(uint64_t handle);
  ParticleManagerData* data{};
};


class ParticleSystem
{
public:
  void InitScene(Scene& scene);
  void Update(Scene& scene, Timestep timestep);

private:
};