#include "EnginePCH.h"

#include "ParticleSystem.h"
#include "Scene.h"
#include "Components.h"
#include <execution>
#include <algorithm>
#include "utilities.h"
#include "Renderer.h"

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

    // set a few variables in a few buffers... (emitter and particle updates can be swapped, probably)
    emitter.particleBuffer->Bind<GFX::Target::SSBO>(0);
    emitter.freeStackBuffer->Bind<GFX::Target::SSBO>(1);

    emitter.timer += dt;
    unsigned particlesToSpawn = 0;
    while (emitter.timer > emitter.interval)
    {
      particlesToSpawn++;
      emitter.timer -= emitter.interval;
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
      emitter_shader->setFloat("u_time", timer.elapsed());
      emitter_shader->setMat4("u_model", transform.GetModel());
      emitter_shader->setFloat("u_emitter.minLife", emitter.minLife);
      emitter_shader->setFloat("u_emitter.maxLife", emitter.maxLife);
      emitter_shader->setVec3("u_emitter.minParticleOffset", emitter.minParticleOffset);
      emitter_shader->setVec3("u_emitter.maxParticleOffset", emitter.maxParticleOffset);
      emitter_shader->setVec3("u_emitter.minParticleVelocity", emitter.minParticleVelocity);
      emitter_shader->setVec3("u_emitter.maxParticleVelocity", emitter.maxParticleVelocity);
      emitter_shader->setVec3("u_emitter.minParticleAccel", emitter.minParticleAccel);
      emitter_shader->setVec3("u_emitter.maxParticleAccel", emitter.maxParticleAccel);
      emitter_shader->setVec2("u_emitter.minParticleScale", emitter.minParticleScale);
      emitter_shader->setVec2("u_emitter.maxParticleScale", emitter.maxParticleScale);
      emitter_shader->setVec4("u_emitter.minParticleColor", emitter.minParticleColor);
      emitter_shader->setVec4("u_emitter.maxParticleColor", emitter.maxParticleColor);
#pragma endregion setting uniforms

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

    emitter.particleBuffer->Bind<GFX::Target::SSBO>(0);
    emitter.freeStackBuffer->Bind<GFX::Target::SSBO>(1);

    int numGroupsa = (emitter.maxParticles + localSize - 1) / localSize;
    glDispatchCompute(numGroupsa, 1, 1);
  }
  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}