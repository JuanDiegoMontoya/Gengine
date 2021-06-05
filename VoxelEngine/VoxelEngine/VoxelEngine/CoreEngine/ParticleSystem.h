#pragma once
#include "StaticBuffer.h"
#include "Texture2D.h"
#include <memory>

class Scene;

class ParticleManager
{
public:
  static uint64_t MakeParticleEmitter(uint32_t maxp, std::string tex);
  static void DestroyParticleEmitter(uint64_t handle);

private:
  friend class ParticleSystem;
  friend class Renderer;

  struct InternalEmitterData
  {
    std::unique_ptr<GFX::StaticBuffer> particleBuffer{};
    std::unique_ptr<GFX::StaticBuffer> freeStackBuffer{};
    std::unique_ptr<GFX::StaticBuffer> indirectDrawBuffer{};
    std::unique_ptr<GFX::StaticBuffer> indicesBuffer{};
    std::unique_ptr<GFX::Texture2D> texture{};
    uint32_t maxParticles{}; // const
    uint32_t numParticles{};
    float timer{ 0.0f };
  };

  static inline uint64_t curHandle_{};
  static inline std::unordered_map<uint64_t, std::unique_ptr<InternalEmitterData>> handleToGPUParticleData_;
};


class ParticleSystem
{
public:
  void Update(Scene& scene, float dt);

private:
};