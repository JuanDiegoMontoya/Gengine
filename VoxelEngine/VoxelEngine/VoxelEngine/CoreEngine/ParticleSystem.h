#pragma once

class Scene;
struct ParticleManagerData;

namespace GFX
{
  class StaticBuffer;
  class Texture2D;
}

class ParticleManager
{
public:
  static ParticleManager& Get();
  uint64_t MakeParticleEmitter(uint32_t maxp, const char* tex);
  void DestroyParticleEmitter(uint64_t handle);

private:
  friend class ParticleSystem;
  friend class Renderer;

  ParticleManager();
  void BindEmitter(uint64_t handle);
  ParticleManagerData* data{};
};


class ParticleSystem
{
public:
  void InitScene(Scene& scene);
  void Update(Scene& scene, float dt);

private:
};