#include "EnginePCH.h"

#include "ParticleSystem.h"
#include "Scene.h"
#include "Components.h"
#include <execution>
#include <algorithm>
#include "utilities.h"
#include <random>
#include "Fence.h"

static thread_local uint64_t x = 123456789, y = 362436069, z = 521288629;

uint64_t xorshf96()
{
  uint64_t t;
  x ^= x << 16;
  x ^= x >> 5;
  x ^= x << 1;

  t = x;
  x = y;
  y = z;
  z = t ^ x ^ y;

  return z;
}

double rng(double low, double high)
{
  uint64_t big = 1023ull << 52ull | xorshf96() & 0xfffffffffffffull;
  double dub = *reinterpret_cast<double*>(&big) - 1.0;
  return Utils::map(dub, 0.0, 1.0, low, high);
}


void ParticleSystem::Update(Scene& scene, float dt)
{
  // TODO: make the fence after rendering commands are issued
  GFX::Fence fence;
  fence.Sync();

  using namespace Components;
  auto view = scene.GetRegistry().view<ParticleEmitter>();
  std::for_each(std::execution::seq, view.begin(), view.end(),
    [&view, dt](entt::entity entity)
    {
      auto& emitter = view.get<ParticleEmitter>(entity);

      // update emitter
      emitter.timer -= dt;
      //printf("Timer: %f\n", emitter.timer);
      while (emitter.timer < 0.0f)
      {
        emitter.timer += emitter.interval;
        if (emitter.numParticles < emitter.maxParticles)
        {
          // spawn particle
          int index;
          if (!emitter.freedIndices.empty())
          {
            index = emitter.freedIndices.front();
            emitter.freedIndices.pop();
            //printf("Spawned particle A\n");
          }
          else
          {
            index = emitter.numParticles;
            //printf("Spawned particle B\n");
          }

          emitter.numParticles++;

          auto& particle = emitter.particles[index];
          particle.life = rng(emitter.minLife, emitter.maxLife);
          //printf("life: %f\n", particle.life);
          particle.alive = true;
          particle.pos.x = rng(emitter.minParticleOffset.x, emitter.maxParticleOffset.x);
          particle.pos.y = rng(emitter.minParticleOffset.y, emitter.maxParticleOffset.y);
          particle.pos.z = rng(emitter.minParticleOffset.z, emitter.maxParticleOffset.z);
          particle.velocity.x = rng(emitter.minParticleVelocity.x, emitter.maxParticleVelocity.x);
          particle.velocity.y = rng(emitter.minParticleVelocity.y, emitter.maxParticleVelocity.y);
          particle.velocity.z = rng(emitter.minParticleVelocity.z, emitter.maxParticleVelocity.z);
          particle.accel.x = rng(emitter.minParticleAccel.x, emitter.maxParticleAccel.x);
          particle.accel.y = rng(emitter.minParticleAccel.y, emitter.maxParticleAccel.y);
          particle.accel.z = rng(emitter.minParticleAccel.z, emitter.maxParticleAccel.z);
          particle.scale.x = rng(emitter.minParticleScale.x, emitter.maxParticleScale.x);
          particle.scale.y = rng(emitter.minParticleScale.y, emitter.maxParticleScale.y);

          particle.color = glm::vec4(rng(0, 1), rng(0, 1), rng(0, 1), rng(0, 1));
        }
      }

      // update individual particles
      for (int i = 0; i < emitter.numParticles; i++)
      {
        auto& particle = emitter.particles[i];

        if (particle.alive)
        {
          particle.life -= dt;
          particle.velocity += particle.accel * dt;
          particle.pos += particle.velocity * dt;

          if (particle.life < 0)
          {
            //printf("Killed particle\n");
            particle.color.a = 0;
            particle.alive = false;
            emitter.numParticles--;
            emitter.freedIndices.push(i);
          }
        }
      }
    });
}
