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

layout(std430, binding = 0) buffer ParticlesShared
{
  ParticleSharedData particlesShared[];
};

layout(std430, binding = 1) buffer ParticlesUpdate
{
  ParticleUpdateData particlesUpdate[];
};

layout(std430, binding = 2) buffer ParticlesRender
{
  ParticleRenderData particlesRender[];
};

layout(std430, binding = 3) coherent buffer stack
{
  int freeCount;
  int indices[];
};

layout(location = 0) uniform int u_particlesToSpawn;
layout(location = 1) uniform vec3 u_seed;
layout(location = 2) uniform vec3 u_pos;
layout(location = 3) uniform EmitterSettings u_emitter; // also uses the next 11 uniform locations (12 total)


float map(float val, float r1s, float r1e, float r2s, float r2e)
{
  return (val - r1s) / (r1e - r1s) * (r2e - r2s) + r2s;
}
float umap(float val, float rs, float re) // [0, 1] -> [rs, re]
{
  return val * (re - rs) + rs;
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
  tseed += 1.61803399;
  return clamp(map(gold_noise(gl_GlobalInvocationID.xy, cos(u_seed.y * tseed)), 0.0, 1.0, low, high), low, high);
}
float rng_s(float low, float high, float seed)
{
  //tseed += rng(0.0, 1.0);
  tseed += 1.61803399;
  //return umap(gold_noise(gl_GlobalInvocationID.xy * u_seed.xy, u_seed.z * tseed * seed), low, high);
  return umap(rand(gl_GlobalInvocationID.xy * u_seed.xy + u_seed.z * tseed * seed), low, high);
}
vec2 rng(vec2 low, vec2 high)
{
  return vec2(rng(low.x, high.x), rng(low.y, high.y));
}
vec3 rng(vec3 low, vec3 high)
{
  //return vec3(rng(low.xy, high.xy), rng(low.z, high.z));
  return vec3(
    rng_s(low.x, high.x, 1.989),
    rng_s(low.y, high.y, 20.233),
    rng_s(low.z, high.z, 3.719)
  );
}
vec4 rng(vec4 low, vec4 high)
{
  return vec4(rng(low.xyz, high.xyz), rng(low.w, high.w));
}

void MakeParticle(
  out ParticleSharedData psd,
  out ParticleUpdateData pud,
  out ParticleRenderData prd)
{
  vec3 velocity = rng(u_emitter.minParticleVelocity.xyz, u_emitter.maxParticleVelocity.xyz);
  vec3 acceleration = rng(u_emitter.minParticleAccel.xyz, u_emitter.maxParticleAccel.xyz);
  float life = rng(u_emitter.minLife, u_emitter.maxLife);
  pud.velocity_acceleration_L.x = packHalf2x16(velocity.xy);
  pud.velocity_acceleration_L.y = packHalf2x16(vec2(velocity.z, acceleration.x));
  pud.velocity_acceleration_L.z = packHalf2x16(acceleration.yz);
  pud.velocity_acceleration_L.w = floatBitsToUint(life);

  vec2 scale = rng(u_emitter.minParticleScale.xy, u_emitter.maxParticleScale.xy);
  vec4 color = rng(u_emitter.minParticleColor.rgba, u_emitter.maxParticleColor.rgba);
  //prd.scale.xy = scale;
  //prd.color = color;
  prd.packedScaleX_packedColorY.x = packHalf2x16(scale);
  prd.packedScaleX_packedColorY.y = packUnorm4x8(color);

  vec3 pos = rng(u_emitter.minParticleOffset, u_emitter.maxParticleOffset);
  //psd.position_A.xyz = (u_model * vec4(pos, 1.0)).xyz;
  psd.position_A.xyz = u_pos + pos;
  psd.position_A.w = 1.0;
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

  MakeParticle(
    particlesShared[indices[indexIndex]],
    particlesUpdate[indices[indexIndex]],
    particlesRender[indices[indexIndex]]
  );
}