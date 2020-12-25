#version 460 core

#include "particle.h"
#include "DrawArraysCommand.h"
layout (std430, binding = 0) buffer data
{
  Particle particles[];
};

layout (std430, binding = 1) coherent buffer stack
{
  DrawArraysCommand cmd;
  int freeCount;
  uint indices[];
};

layout (std430, binding = 2) buffer emitter
{
  coherent uint numParticles;
  readonly int maxParticles;
  readonly float minLife;
  readonly float maxLife;
  readonly vec4 minParticleOffset;
  readonly vec4 maxParticleOffset;
  readonly vec4 minParticleVelocity;
  readonly vec4 maxParticleVelocity;
  readonly vec4 minParticleAccel;
  readonly vec4 maxParticleAccel;
  readonly vec2 minParticleScale;
  readonly vec2 maxParticleScale;
  readonly vec4 minParticleColor;
  readonly vec4 maxParticleColor;
}

uniform int u_particlesToSpawn;
uniform float u_time;
uniform mat4 u_model;

float map(float val, float r1s, float r1e, float r2s, float r2e)
{
  return (val - r1s) / (r1e - r1s) * (r2e - r2s) + r2s;
}
vec2 map(vec2 val, vec2 r1s, vec2 r1e, vec2 r2s, vec2 r2e)
{
  return (val - r1s) / (r1e - r1s) * (r2e - r2s) + r2s;
}
vec3 map(vec3 val, vec3 r1s, vec3 r1e, vec3 r2s, vec3 r2e)
{
  return (val - r1s) / (r1e - r1s) * (r2e - r2s) + r2s;
}
vec4 map(vec4 val, vec4 r1s, vec4 r1e, vec4 r2s, vec4 r2e)
{
  return (val - r1s) / (r1e - r1s) * (r2e - r2s) + r2s;
}
float tseed = 0.0;
float rng(float low, float high)
{
  tseed += 1.0;
  return map(gold_noise(gl_GlobalInvocationID.xy, u_time + tseed), 0.0, 1.0, low, high);
}
vec2 rng(vec2 low, vec2 high)
{
  return vec2(rng(low.x, high.x), rng(low.y, high.y));
}
vec3 rng(vec3 low, vec3 high)
{
  return vec3(rng(low.xy, high.xy), rng(low.z, high.z));
}
vec4 rng(vec4 low, vec4 high)
{
  return vec4(rng(low.xyz, high.xyz), rng(low.w, high.w));
}

void MakeParticle(out Particle particle)
{
  // particle.velocity.x = rng(minParticleVelocity.x, maxParticleVelocity.x);
  // particle.velocity.y = rng(minParticleVelocity.y, maxParticleVelocity.y);
  // particle.velocity.z = rng(minParticleVelocity.z, maxParticleVelocity.z);
  // particle.accel.x = rng(minParticleAccel.x, maxParticleAccel.x);
  // particle.accel.y = rng(minParticleAccel.y, maxParticleAccel.y);
  // particle.accel.z = rng(minParticleAccel.z, maxParticleAccel.z);
  // particle.scale.x = rng(minParticleScale.x, maxParticleScale.x);
  // particle.scale.y = rng(minParticleScale.y, maxParticleScale.y);
  // particle.color.r = rng(minParticleColor.r, maxParticleColor.r);
  // particle.color.g = rng(minParticleColor.g, maxParticleColor.g);
  // particle.color.b = rng(minParticleColor.b, maxParticleColor.b);
  // particle.color.a = rng(minParticleColor.a, maxParticleColor.a);
  // particle.pos.x = rng(minParticleOffset.x, maxParticleOffset.x);
  // particle.pos.y = rng(minParticleOffset.y, maxParticleOffset.y);
  // particle.pos.z = rng(minParticleOffset.z, maxParticleOffset.z);
  // glm::mat4 md = glm::translate(glm::scale(glm::mat4(1), transform.GetScale()), transform.GetTranslation());
  particle.life = rng(minLife, maxLife);
  particle.alive = 1;
  particle.velocity.xyz = rng(minParticleVelocity.xyz, maxParticleVelocity.xyz);
  particle.accel.xyz = rng(minParticleAccel.xyz, maxParticleAccel.xyz);
  particle.scale.xy = rng(minParticleScale.xy, maxParticleScale.xy);
  particle.color.rgba = rng(minParticleColor.rgba, maxParticleColor.rgba);
  particle.pos.xyz = rng(minParticleOffset.xyz, maxParticleOffset.xyz);
  particle.pos.w = 1.0f;
  particle.pos = u_model * particle.pos;
}

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;
void main()
{
  uint start = gl_GlobalInvocationID.x;
  uint stride = gl_NumWorkGroups.x * gl_WorkGroupSize.x;

  for (uint i = start; i < u_particlesToSpawn; i += stride)
  {
    // undo increment and return if max particles reached
    int num = atomicAdd(numParticles, 1);
    if (num > maxParticles)
    {
      atomicAdd(numParticles, -1);
      return;
    }

    // undo decrement and use end of array if nothing in freelist
    int newParticleIndex = atomicAdd(freeCount, -1);
    if (newParticleIndex < 0)
    {
      atomicAdd(freeCount, 1);
      newParticleIndex = num;
    }

    MakeParticle(particles[newParticleIndex]);
  }
}