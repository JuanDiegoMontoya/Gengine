#include "../../PCH.h"

#include <execution>
#include <algorithm>
#include <bit>
#include <memory>
#include <unordered_map>

#include <engine/gfx/resource/ShaderManager.h>
#include <engine/gfx/resource/TextureManager.h>

#include "ParticleSystem.h"
#include "../../Scene.h"
#include "../../utilities.h"
#include <engine/gfx/Renderer.h>

#include "../component/ParticleEmitter.h"
#include "../component/Transform.h"
#include <engine/gfx/api/Buffer.h>
#include <engine/gfx/api/DebugMarker.h>
#include <engine/gfx/api/Indirect.h>
#include <engine/gfx/api/Fence.h>
#include <engine/gfx/Camera.h>

#define LOG_EMITTER_UPDATE_TIME 0
#define LOG_PARTICLE_UPDATE_TIME 0

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

  struct ParticleSharedData
  {
    glm::vec4 position{ 0 };
  };

  struct ParticleUpdateData
  {
    //glm::vec4 velocity_L{ 0 }; // .w = life
    //glm::vec4 acceleration{ 0 };

    // unpackHalf2x16 x 3 to access vel/accel, uintBitsToFloat(.w) to access L
    glm::uvec4 velocity_acceleration_L{ 0 };
  };

  struct ParticleRenderData
  {
    //glm::vec4 color{ 0 };
    //glm::vec4 scale{ 1 };
    glm::uvec2 packedScaleX_packedColorY = { glm::packHalf2x16({ 1, 1 }), glm::packUnorm4x8(glm::vec4(0)) };
  };
}

struct InternalEmitterData
{
  std::optional<GFX::Buffer> particleSharedDataBuffer{};
  std::optional<GFX::Buffer> particleUpdateDataBuffer{};
  std::optional<GFX::Buffer> particleRenderDataBuffer{};
  std::optional<GFX::Buffer> freeStackBuffer{};
  std::optional<GFX::Buffer> indirectDrawBuffer{};
  std::optional<GFX::Buffer> indicesBuffer{};
  std::optional<GFX::TextureView> textureView;
  std::optional<GFX::TextureSampler> textureSampler;
  uint32_t maxParticles{}; // const
  uint32_t numParticles{};
  float timer{ 0.0f };
};

struct ParticleManagerData
{
  uint64_t curHandle_{};
  std::unordered_map<uint64_t, std::unique_ptr<InternalEmitterData>> handleToGPUParticleData_;
};

static void OnEmitterConstruct([[maybe_unused]] entt::basic_registry<entt::entity>& registry, entt::entity entity)
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

void ParticleSystem::Update(Scene& scene, Timestep timestep)
{
  GFX::DebugMarker marker("Particle system update");
  static Timer timer;

  using namespace Component;
  auto view = scene.GetRegistry().view<ParticleEmitter, Transform>();

  {
#if LOG_EMITTER_UPDATE_TIME
    GFX::TimerQuery timerQuery;
#endif
    GFX::DebugMarker emitterMarker("Update emitters");
    auto emitter_shader = GFX::ShaderManager::GetShader("update_particle_emitter");
    emitter_shader->Bind();
    for (entt::entity entity : view)
    {
      auto [emitter, transform] = view.get<ParticleEmitter, Transform>(entity);
      auto& emitterData = ParticleManager::Get().data->handleToGPUParticleData_[emitter.handle];
      ASSERT(emitterData);

      emitterData->timer += timestep.dt_effective;
      //unsigned particlesToSpawn = 0;
      //while (emitterData->timer > emitter.data.interval)
      //{
      //  particlesToSpawn++;
      //  emitterData->timer -= emitter.data.interval;
      //}
      unsigned particlesToSpawn = emitterData->timer / emitter.data.interval;
      //particlesToSpawn = glm::min(particlesToSpawn, emitterData.maxParticles - emitterData.numParticles);
      //emitter.numParticles += particlesToSpawn;
      emitterData->timer = glm::mod(emitterData->timer, emitter.data.interval);
      ASSERT(particlesToSpawn >= 0);

      // update emitter
      if (particlesToSpawn > 0)
      {
        // set a few variables in a few buffers... (emitter and particle updates can be swapped, probably)
        emitterData->particleSharedDataBuffer->Bind<GFX::Target::SHADER_STORAGE_BUFFER>(0);
        emitterData->particleUpdateDataBuffer->Bind<GFX::Target::SHADER_STORAGE_BUFFER>(1);
        emitterData->particleRenderDataBuffer->Bind<GFX::Target::SHADER_STORAGE_BUFFER>(2);
        emitterData->freeStackBuffer->Bind<GFX::Target::SHADER_STORAGE_BUFFER>(3);

#pragma region uniforms
        emitter_shader->SetInt("u_particlesToSpawn", particlesToSpawn);
        //emitter_shader->SetFloat("u_time", static_cast<float>(timer.Elapsed()) + 1.61803f);
        emitter_shader->SetVec3("u_seed", { rng(.1, .9), rng(.1, .9), rng(.1, .9) });
        //emitter_shader->SetMat4("u_model", transform.GetModel());
        emitter_shader->SetVec3("u_pos", transform.GetTranslation());
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

        const int emitterUpdateGroupSize = 64; // maybe should query shader for this value
        int numGroups = (particlesToSpawn + emitterUpdateGroupSize - 1) / emitterUpdateGroupSize;
        glDispatchCompute(numGroups, 1, 1);
      }
    }
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
#if LOG_EMITTER_UPDATE_TIME
    printf("Emitter update time: %f ms\n", (double)timerQuery.Elapsed_ns() / 1000000.0);
#endif
  }

  // update particles in the emitter
  {
#if LOG_PARTICLE_UPDATE_TIME
    GFX::TimerQuery timerQuery;
#endif
    GFX::DebugMarker particleMarker("Update particle dynamic state");
    auto particle_shader = GFX::ShaderManager::GetShader("update_particle");
    particle_shader->Bind();
    particle_shader->SetFloat("u_dt", timestep.dt_effective);

    // TODO: hackity hack hack, move to particle rendering in a new compute shader just for culling
    // it will be slower, but offers more flexibility
    const auto* mainRenderViewHack = GFX::Renderer::GetMainRenderView();
    particle_shader->SetVec3("u_viewPos", mainRenderViewHack->camera->viewInfo.position);
    particle_shader->SetVec3("u_forwardDir", mainRenderViewHack->camera->viewInfo.GetForwardDir());
    
    for (entt::entity entity : view)
    {
      auto [emitter, transform] = view.get<ParticleEmitter, Transform>(entity);
      auto& emitterData = ParticleManager::Get().data->handleToGPUParticleData_[emitter.handle];
      ASSERT(emitterData);

      uint32_t zero{ 0 };
      glClearNamedBufferSubData(emitterData->indirectDrawBuffer->GetAPIHandle(), GL_R32UI, offsetof(DrawArraysIndirectCommand, instanceCount),
        sizeof(GLuint), GL_RED, GL_UNSIGNED_INT, &zero);

      emitterData->particleSharedDataBuffer->Bind<GFX::Target::SHADER_STORAGE_BUFFER>(0);
      emitterData->particleUpdateDataBuffer->Bind<GFX::Target::SHADER_STORAGE_BUFFER>(1);
      emitterData->particleRenderDataBuffer->Bind<GFX::Target::SHADER_STORAGE_BUFFER>(2);
      emitterData->freeStackBuffer->Bind<GFX::Target::SHADER_STORAGE_BUFFER>(3);
      emitterData->indirectDrawBuffer->Bind<GFX::Target::SHADER_STORAGE_BUFFER>(4);
      emitterData->indicesBuffer->Bind<GFX::Target::SHADER_STORAGE_BUFFER>(5);

      int particleUpdateGroupSize = 1 * 128;
      int numGroups = (emitterData->maxParticles + particleUpdateGroupSize - 1) / particleUpdateGroupSize;
      glDispatchCompute(numGroups, 1, 1);
    }
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
#if LOG_PARTICLE_UPDATE_TIME
    static double asdf = 0;
    asdf = (double)timerQuery.Elapsed_ns() / 1000000.0 * .01 + .99 * asdf;
    printf("Particle update time: %f ms\n", asdf);
#endif
  }

  // reset timer every 10 seconds to avoid precision issues
  if (timer.Elapsed() > 10.0)
  {
    timer.Reset();
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

  GFX::BindTextureView(0, *emitterData->textureView, *emitterData->textureSampler);
  emitterData->particleSharedDataBuffer->Bind<GFX::Target::SHADER_STORAGE_BUFFER>(0);
  emitterData->particleUpdateDataBuffer->Bind<GFX::Target::SHADER_STORAGE_BUFFER>(1);
  emitterData->particleRenderDataBuffer->Bind<GFX::Target::SHADER_STORAGE_BUFFER>(2);
  emitterData->indicesBuffer->Bind<GFX::Target::SHADER_STORAGE_BUFFER>(3);
  emitterData->indirectDrawBuffer->Bind<GFX::Target::DRAW_INDIRECT_BUFFER>();
}

uint64_t ParticleManager::MakeParticleEmitter(uint32_t maxp, const GFX::TextureView& texView, const GFX::TextureSampler& sampler)
{
  uint64_t newHandle = ++data->curHandle_;

  auto& newEmitter = (data->handleToGPUParticleData_[newHandle] = std::make_unique<InternalEmitterData>());
  newEmitter->maxParticles = maxp;

  auto tps = std::make_unique<ParticleSharedData[]>(maxp);
  auto tpu = std::make_unique<ParticleUpdateData[]>(maxp);
  auto tpr = std::make_unique<ParticleRenderData[]>(maxp);
  newEmitter->particleSharedDataBuffer = GFX::Buffer::Create(std::span(tps.get(), maxp));
  newEmitter->particleUpdateDataBuffer = GFX::Buffer::Create(std::span(tpu.get(), maxp));
  newEmitter->particleRenderDataBuffer = GFX::Buffer::Create(std::span(tpr.get(), maxp));

  const size_t bytes = sizeof(int32_t) + maxp * sizeof(int32_t);
  uint8_t* mem = new uint8_t[bytes];
  reinterpret_cast<int32_t&>(mem[0]) = maxp;
  int32_t val = 0;
  for (size_t i = sizeof(int32_t); i < bytes; i += sizeof(int32_t))
  {
    reinterpret_cast<int32_t&>(mem[i]) = val++;
  }
  newEmitter->freeStackBuffer = GFX::Buffer::Create(std::span(mem, bytes));
  delete[] mem;

  DrawArraysIndirectCommand cmd
  {
    .count = 4,
    .instanceCount = 0,
    .first = 0,
    .baseInstance = 0
  };
  newEmitter->indirectDrawBuffer = GFX::Buffer::Create(std::span(&cmd, 1));
  newEmitter->indicesBuffer = GFX::Buffer::Create(sizeof(GLuint) * maxp);

  newEmitter->textureView = texView;
  newEmitter->textureSampler = sampler;

  return newHandle;
}

uint64_t ParticleManager::MakeParticleEmitter(uint32_t maxp, hashed_string textureName)
{
  GFX::Texture* texture = GFX::TextureManager::GetTexture(textureName);
  if (!texture)
  {
    return 0;
  }

  GFX::SamplerState defaultSamplerState{};
  defaultSamplerState.asBitField.magFilter = GFX::Filter::NEAREST;
  defaultSamplerState.asBitField.minFilter = GFX::Filter::NEAREST;
  return MakeParticleEmitter(maxp,
    *GFX::TextureView::Create(*texture),
    *GFX::TextureSampler::Create(defaultSamplerState));
}

void ParticleManager::DestroyParticleEmitter(uint64_t handle)
{
  if (handle == 0) return;
  auto it = data->handleToGPUParticleData_.find(handle);
  ASSERT(it != data->handleToGPUParticleData_.end());
  data->handleToGPUParticleData_.erase(it);
}
