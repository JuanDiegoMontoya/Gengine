#include "PCH.h"

#include <execution>
#include <algorithm>
#include <bit>
#include <memory>
#include <unordered_map>

#include "ShaderManager.h"

#include "ParticleSystem.h"
#include "Scene.h"
#include "utilities.h"
#include "Renderer.h"

#include "Components/ParticleEmitter.h"
#include "Components/Transform.h"
#include "StaticBuffer.h"
#include "Texture2D.h"
#include "DebugMarker.h"

namespace
{
  static thread_local uint64_t x = 123456789, y = 362436069, z = 521288629;

  uint64_t xorshf96()
  {
    x ^= x << 16;
    x ^= x >> 5;
    x ^= x << 1;

    uint64_t t = x;
    x = y;
    y = z;
    z = t ^ x ^ y;

    return z;
  }

  double rng()
  {
    uint64_t bits = 1023ull << 52ull | xorshf96() & 0xfffffffffffffull;
    return std::bit_cast<double>(bits) - 1.0;
  }

  double rng(double low, double high)
  {
    return Utils::map(rng(), 0.0, 1.0, low, high);
  }

  // TODO: split into different structs based on usage (SoA style)
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

}

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

struct ParticleManagerData
{
  uint64_t curHandle_{};
  std::unordered_map<uint64_t, std::unique_ptr<InternalEmitterData>> handleToGPUParticleData_;
};

static void OnEmitterConstruct(entt::basic_registry<entt::entity>& registry, entt::entity entity)
{
  printf("Constructed particle emitter on entity %d\n", entity);
}

static void OnEmitterDestroy(entt::basic_registry<entt::entity>& registry, entt::entity entity)
{
  printf("Destroyed particle emitter on entity %d\n", entity);
  auto& emitter = registry.get<Component::ParticleEmitter>(entity);
  ParticleManager::Get().DestroyParticleEmitter(emitter.handle);
}

void ParticleSystem::InitScene(Scene& scene)
{
  scene.GetRegistry().on_construct<Component::ParticleEmitter>()
    .connect<&OnEmitterConstruct>();
  scene.GetRegistry().on_destroy<Component::ParticleEmitter>()
    .connect<&OnEmitterDestroy>();
}

void ParticleSystem::Update(Scene& scene, float dt)
{
  GFX::DebugMarker marker("Particle system update");
  static Timer timer;
  const int localSize = 128; // maybe should query shader for this value

  using namespace Component;
  auto view = scene.GetRegistry().view<ParticleEmitter, Transform>();

  {
    GFX::DebugMarker emitterMarker("Update emitters");
    auto emitter_shader = GFX::ShaderManager::Get()->GetShader("update_particle_emitter");
    emitter_shader->Bind();
    for (entt::entity entity : view)
    {
      auto [emitter, transform] = view.get<ParticleEmitter, Transform>(entity);
      auto& emitterData = ParticleManager::Get().data->handleToGPUParticleData_[emitter.handle];
      ASSERT(emitterData);

      // set a few variables in a few buffers... (emitter and particle updates can be swapped, probably)
      emitterData->particleBuffer->Bind<GFX::Target::SHADER_STORAGE_BUFFER>(0);
      emitterData->freeStackBuffer->Bind<GFX::Target::SHADER_STORAGE_BUFFER>(1);

      emitterData->timer += dt;
      unsigned particlesToSpawn = 0;
      while (emitterData->timer > emitter.data.interval)
      {
        particlesToSpawn++;
        emitterData->timer -= emitter.data.interval;
      }
      //unsigned particlesToSpawn = glm::floor(emitter.timer / emitter.interval);
      //particlesToSpawn = glm::min(particlesToSpawn, emitter.maxParticles - emitter.numParticles);
      //emitter.numParticles += particlesToSpawn;
      //emitter.timer = glm::mod(emitter.timer, emitter.interval);
      ASSERT(particlesToSpawn >= 0);

      // update emitter
      if (particlesToSpawn > 0)
      {
#pragma region uniforms
        emitter_shader->SetInt("u_particlesToSpawn", particlesToSpawn);
        emitter_shader->SetFloat("u_time", static_cast<float>(timer.elapsed()) + 1.61803f);
        emitter_shader->SetMat4("u_model", transform.GetModel());
        emitter_shader->SetFloat("u_emitter.minLife", emitter.data.minLife);
        emitter_shader->SetFloat("u_emitter.maxLife", emitter.data.maxLife);
        emitter_shader->SetVec3("u_emitter.minParticleOffset", emitter.data.minParticleOffset);
        emitter_shader->SetVec3("u_emitter.maxParticleOffset", emitter.data.maxParticleOffset);
        emitter_shader->SetVec3("u_emitter.minParticleVelocity", emitter.data.minParticleVelocity);
        emitter_shader->SetVec3("u_emitter.maxParticleVelocity", emitter.data.maxParticleVelocity);
        emitter_shader->SetVec3("u_emitter.minParticleAccel", emitter.data.minParticleAccel);
        emitter_shader->SetVec3("u_emitter.maxParticleAccel", emitter.data.maxParticleAccel);
        emitter_shader->SetVec2("u_emitter.minParticleScale", emitter.data.minParticleScale);
        emitter_shader->SetVec2("u_emitter.maxParticleScale", emitter.data.maxParticleScale);
        emitter_shader->SetVec4("u_emitter.minParticleColor", emitter.data.minParticleColor);
        emitter_shader->SetVec4("u_emitter.maxParticleColor", emitter.data.maxParticleColor);
#pragma endregion

        int numGroups = (particlesToSpawn + localSize - 1) / localSize;
        glDispatchCompute(numGroups, 1, 1);
      }
    }
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
  }

  // update particles in the emitter
  {
    GFX::DebugMarker particleMarker("Update particle dynamic state");
    auto particle_shader = GFX::ShaderManager::Get()->GetShader("update_particle");
    particle_shader->Bind();
    particle_shader->SetFloat("u_dt", dt);

    for (entt::entity entity : view)
    {
      auto [emitter, transform] = view.get<ParticleEmitter, Transform>(entity);
      auto& emitterData = ParticleManager::Get().data->handleToGPUParticleData_[emitter.handle];
      ASSERT(emitterData);

      GLuint zero{ 0 };
      glClearNamedBufferSubData(emitterData->indirectDrawBuffer->GetID(), GL_R32UI, offsetof(DrawArraysIndirectCommand, instanceCount),
        sizeof(GLuint), GL_RED, GL_UNSIGNED_INT, &zero);

      emitterData->particleBuffer->Bind<GFX::Target::SHADER_STORAGE_BUFFER>(0);
      emitterData->freeStackBuffer->Bind<GFX::Target::SHADER_STORAGE_BUFFER>(1);
      emitterData->indirectDrawBuffer->Bind<GFX::Target::SHADER_STORAGE_BUFFER>(2);
      emitterData->indicesBuffer->Bind<GFX::Target::SHADER_STORAGE_BUFFER>(3);

      int numGroups = (emitterData->maxParticles + localSize - 1) / localSize;
      glDispatchCompute(numGroups, 1, 1);
    }
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
  }

  // reset timer every 10 seconds to avoid precision issues
  if (timer.elapsed() > 10.0)
  {
    timer.reset();
  }
}

ParticleManager& ParticleManager::Get()
{
  static ParticleManager manager;
  return manager;
}

ParticleManager::ParticleManager()
{
  data = new ParticleManagerData;
}

void ParticleManager::BindEmitter(uint64_t handle)
{
  auto& emitterData = data->handleToGPUParticleData_[handle];

  emitterData->texture->Bind(0);
  emitterData->particleBuffer->Bind<GFX::Target::SHADER_STORAGE_BUFFER>(0);
  emitterData->indicesBuffer->Bind<GFX::Target::SHADER_STORAGE_BUFFER>(1);
  emitterData->indirectDrawBuffer->Bind<GFX::Target::DRAW_INDIRECT_BUFFER>();
}

uint64_t ParticleManager::MakeParticleEmitter(uint32_t maxp, const char* tex)
{
  uint64_t newHandle = ++data->curHandle_;

  auto& newEmitter = (data->handleToGPUParticleData_[newHandle] = std::make_unique<InternalEmitterData>());
  newEmitter->maxParticles = maxp;

  auto tp = std::make_unique<Particle[]>(maxp);
  newEmitter->particleBuffer = std::make_unique<GFX::StaticBuffer>(tp.get(), sizeof(Particle) * maxp, GFX::BufferFlag::NONE);

  const size_t bytes = sizeof(int32_t) + maxp * sizeof(int32_t);
  uint8_t* mem = new uint8_t[bytes];
  reinterpret_cast<int32_t&>(mem[0]) = maxp;
  int32_t val = 0;
  for (size_t i = sizeof(int32_t); i < bytes; i += sizeof(int32_t))
  {
    reinterpret_cast<int32_t&>(mem[i]) = val++;
  }
  newEmitter->freeStackBuffer = std::make_unique<GFX::StaticBuffer>(mem, bytes, GFX::BufferFlag::NONE);
  delete[] mem;

  DrawArraysIndirectCommand cmd
  {
    .count = 4,
    .instanceCount = 0,
    .first = 0,
    .baseInstance = 0
  };
  newEmitter->indirectDrawBuffer = std::make_unique<GFX::StaticBuffer>(&cmd, sizeof(cmd));
  newEmitter->indicesBuffer = std::make_unique<GFX::StaticBuffer>(nullptr, sizeof(GLuint) * maxp);

  newEmitter->texture = std::make_unique<GFX::Texture2D>(tex);

  return newHandle;
}

void ParticleManager::DestroyParticleEmitter(uint64_t handle)
{
  if (handle == 0) return;
  auto it = data->handleToGPUParticleData_.find(handle);
  ASSERT(it != data->handleToGPUParticleData_.end());
  data->handleToGPUParticleData_.erase(it);
}
