#version 460 core

#include "particle.h"
#include "indirect.h.glsl"

#define PARTICLES_PER_THREAD 1
#define CULLING_MIN_ANGLE 0.4 // Lower = more lenient culling. Should not reduce below zero unless huge particles are expected

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

layout(std430, binding = 3) buffer Stack
{
  coherent int freeCount;
  int indices[];
};

layout(std430, binding = 4) coherent buffer IndirectCommand
{
  DrawArraysCommand indirectCommand;
};

layout(std430, binding = 5) buffer Drawindices
{
  uint drawIndices[];
};

layout(location = 0) uniform float u_dt;
layout(location = 1) uniform vec3 u_viewPos;
layout(location = 2) uniform vec3 u_forwardDir;

shared int sh_freeIndex;
shared int sh_requestedFreeIndices;
shared uint sh_drawIndex;
shared uint sh_requestedDrawIndices;

layout(local_size_x = 128, local_size_y = 1, local_size_z = 1) in;
void main()
{
  // strategy: record how many elements we actually need, request it at once, then write to buffers

  for (int i = 0; i < PARTICLES_PER_THREAD; i++)
  {
    if (gl_LocalInvocationIndex == 0)
    {
      sh_requestedFreeIndices = 0;
      sh_requestedDrawIndices = 0;
    }

    barrier();
    memoryBarrierShared();

    int index = int(gl_GlobalInvocationID.x) * PARTICLES_PER_THREAD + i; // linear access
    //int index = int(gl_NumWorkGroups.x * gl_WorkGroupSize.x * uint(i) + gl_LocalInvocationIndex.x * gl_WorkGroupID.x); // strided access
    bool needFreeIndex = false;
    bool needDrawIndex = false;
    if (index < particlesShared.length())
    {
      ParticleSharedData psd = particlesShared[index];
      ParticleUpdateData pud = particlesUpdate[index];
      vec2 unpackMiddle = unpackHalf2x16(pud.velocity_acceleration_L.y);
      vec3 velocity = vec3(unpackHalf2x16(pud.velocity_acceleration_L.x), unpackMiddle.x);
      vec3 acceleration = vec3(unpackMiddle.y, unpackHalf2x16(pud.velocity_acceleration_L.z));
      float life = uintBitsToFloat(pud.velocity_acceleration_L.w);

      if (psd.position_A.w != 0.0)
      {
        velocity += acceleration * u_dt;
        psd.position_A.xyz += velocity * u_dt;
        if (life <= 0.0) // particle just died
        {
          psd.position_A.w = 0.0;

          needFreeIndex = true;
          atomicAdd(sh_requestedFreeIndices, 1);
        }
        else if (dot(u_forwardDir, normalize(psd.position_A.xyz - u_viewPos)) > CULLING_MIN_ANGLE)
        {
          // particle is alive, so we will render it (add its index to drawIndices)
          needDrawIndex = true;
          atomicAdd(sh_requestedDrawIndices, 1);
        }
        life -= u_dt;
      }
      particlesShared[index] = psd;

      pud.velocity_acceleration_L.x = packHalf2x16(velocity.xy);
      pud.velocity_acceleration_L.y = packHalf2x16(vec2(velocity.z, acceleration.x));
      pud.velocity_acceleration_L.z = packHalf2x16(acceleration.yz);
      pud.velocity_acceleration_L.w = floatBitsToUint(life);
      particlesUpdate[index] = pud;
    }

    barrier();
    memoryBarrierShared();

    if (gl_LocalInvocationIndex == 0)
    {
      sh_freeIndex = atomicAdd(freeCount, sh_requestedFreeIndices);
      sh_drawIndex = atomicAdd(indirectCommand.instanceCount, sh_requestedDrawIndices);
    }

    barrier();
    memoryBarrierShared();

    if (needFreeIndex)
    {
      int freeIndex = atomicAdd(sh_freeIndex, 1);
      indices[freeIndex] = index;
    }

    if (needDrawIndex)
    {
      uint drawIndex = atomicAdd(sh_drawIndex, 1);
      drawIndices[drawIndex] = index;
    }

#if PARTICLES_PER_THREAD > 1
    barrier();
    memoryBarrierShared();
#endif
  }
}