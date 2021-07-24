#version 460 core

#include "particle.h"
#include "noise.h"

struct EmitterSettings
{
  float minLife;
  float maxLife;
  vec3 minParticleOffset;
  vec3 maxParticleOffset;
  vec3 minParticleVelocity;
  vec3 maxParticleVelocity;
  vec3 minParticleAccel;
  vec3 maxParticleAccel;
  vec2 minParticleScale;
  vec2 maxParticleScale;
  vec4 minParticleColor;
  vec4 maxParticleColor;
};

layout (std430, binding = 0) buffer data
{
  Particle particles[];
};

layout (std430, binding = 1) coherent buffer stack
{
  int freeCount;
  int indices[];
};

layout (location = 0) uniform int u_particlesToSpawn;
layout (location = 1) uniform float u_time;
layout (location = 2) uniform mat4 u_model;
layout (location = 3) uniform EmitterSettings u_emitter; // also uses the next 11 uniform locations (12 total)

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
  return clamp(map(gold_noise(gl_GlobalInvocationID.xy + u_time, u_time + tseed), 0.0, 1.0, low, high), low, high);
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

Particle MakeParticle()
{
  Particle particle;
  particle.alive = 1;
  particle.life = rng(u_emitter.minLife, u_emitter.maxLife);
  particle.velocity.xyz = rng(u_emitter.minParticleVelocity.xyz, u_emitter.maxParticleVelocity.xyz);
  particle.accel.xyz = rng(u_emitter.minParticleAccel.xyz, u_emitter.maxParticleAccel.xyz);
  particle.scale.xy = rng(u_emitter.minParticleScale.xy, u_emitter.maxParticleScale.xy);
  particle.color.rgba = rng(u_emitter.minParticleColor.rgba, u_emitter.maxParticleColor.rgba);
  vec3 pos = rng(u_emitter.minParticleOffset.xyz, u_emitter.maxParticleOffset.xyz);
  particle.pos = u_model * vec4(pos, 1.0);

  return particle;
}

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;
void main()
{
  uint index = gl_GlobalInvocationID.x;
  if (index >= u_particlesToSpawn)
    return;

  // undo decrement and return if nothing in freelist
  int indexIndex = atomicAdd(freeCount, -1) - 1;
  if (indexIndex < 0)
  {
    atomicAdd(freeCount, 1);
    return;
  }

  particles[indices[indexIndex]] = MakeParticle();
}