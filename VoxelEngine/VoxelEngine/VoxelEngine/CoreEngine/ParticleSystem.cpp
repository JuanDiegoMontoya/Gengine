#include "EnginePCH.h"

#include "ParticleSystem.h"
#include "Scene.h"
#include <execution>
#include <algorithm>
#include "utilities.h"
#include "Renderer.h"

#include "Components/ParticleEmitter.h"
#include "Components/Transform.h"

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
    return *reinterpret_cast<double*>(&bits) - 1.0;
  }

  double rng(double low, double high)
  {
    return Utils::map(rng(), 0.0, 1.0, low, high);
  }

  // TODO: split into different structs based on usage
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


void ParticleSystem::Update(Scene& scene, float dt)
{
  static Timer timer;
  auto& emitter_shader = Shader::shaders["update_particle_emitter"];
  emitter_shader->Use();
  const int localSize = 128; // maybe should query shader for this value
  using namespace Components;
  auto view = scene.GetRegistry().view<ParticleEmitter, Transform>();
  for (entt::entity entity : view)
  {
    auto [emitter, transform] = view.get<ParticleEmitter, Transform>(entity);
    auto& emitterData = ParticleManager::handleToGPUParticleData_[emitter.handle];
    ASSERT(emitterData);
    
    // set a few variables in a few buffers... (emitter and particle updates can be swapped, probably)
    emitterData->particleBuffer->Bind<GFX::Target::SSBO>(0);
    emitterData->freeStackBuffer->Bind<GFX::Target::SSBO>(1);

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
      emitter_shader->setInt("u_particlesToSpawn", particlesToSpawn);
      emitter_shader->setFloat("u_time", static_cast<float>(timer.elapsed()) + 1.61803f);
      emitter_shader->setMat4("u_model", transform.GetModel());
      emitter_shader->setFloat("u_emitter.minLife", emitter.data.minLife);
      emitter_shader->setFloat("u_emitter.maxLife", emitter.data.maxLife);
      emitter_shader->setVec3("u_emitter.minParticleOffset", emitter.data.minParticleOffset);
      emitter_shader->setVec3("u_emitter.maxParticleOffset", emitter.data.maxParticleOffset);
      emitter_shader->setVec3("u_emitter.minParticleVelocity", emitter.data.minParticleVelocity);
      emitter_shader->setVec3("u_emitter.maxParticleVelocity", emitter.data.maxParticleVelocity);
      emitter_shader->setVec3("u_emitter.minParticleAccel", emitter.data.minParticleAccel);
      emitter_shader->setVec3("u_emitter.maxParticleAccel", emitter.data.maxParticleAccel);
      emitter_shader->setVec2("u_emitter.minParticleScale", emitter.data.minParticleScale);
      emitter_shader->setVec2("u_emitter.maxParticleScale", emitter.data.maxParticleScale);
      emitter_shader->setVec4("u_emitter.minParticleColor", emitter.data.minParticleColor);
      emitter_shader->setVec4("u_emitter.maxParticleColor", emitter.data.maxParticleColor);
#pragma endregion

      int numGroups = (particlesToSpawn + localSize - 1) / localSize;
      glDispatchCompute(numGroups, 1, 1);
    }
  }
  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

  // update particles in the emitter
  auto& particle_shader = Shader::shaders["update_particle"];
  particle_shader->Use();
  particle_shader->setFloat("u_dt", dt);

  for (entt::entity entity : view)
  {
    auto [emitter, transform] = view.get<ParticleEmitter, Transform>(entity);
    auto& emitterData = ParticleManager::handleToGPUParticleData_[emitter.handle];
    ASSERT(emitterData);

    GLuint zero{ 0 };
    glClearNamedBufferSubData(emitterData->indirectDrawBuffer->GetID(), GL_R32UI, offsetof(DrawArraysIndirectCommand, instanceCount),
      sizeof(GLuint), GL_RED, GL_UNSIGNED_INT, &zero);

    emitterData->particleBuffer->Bind<GFX::Target::SSBO>(0);
    emitterData->freeStackBuffer->Bind<GFX::Target::SSBO>(1);
    emitterData->indirectDrawBuffer->Bind<GFX::Target::SSBO>(2);
    emitterData->indicesBuffer->Bind<GFX::Target::SSBO>(3);

    int numGroups = (emitterData->maxParticles + localSize - 1) / localSize;
    glDispatchCompute(numGroups, 1, 1);
  }
  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

  // reset timer every 10 seconds to avoid precision issues
  if (timer.elapsed() > 10.0)
  {
    timer.reset();
  }
}

uint64_t ParticleManager::MakeParticleEmitter(uint32_t maxp, std::string tex)
{
  uint64_t newHandle = ++curHandle_;

  auto& newEmitter = (handleToGPUParticleData_[newHandle] = std::make_unique<InternalEmitterData>());
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
  auto it = handleToGPUParticleData_.find(handle);
  ASSERT(it != handleToGPUParticleData_.end());
  handleToGPUParticleData_.erase(it);
}