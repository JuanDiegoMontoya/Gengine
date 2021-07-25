#version 460 core
#include "particle.h"
#include "indirect.h.glsl"

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

shared int sh_freeIndex;
shared int sh_requestedFreeIndices;
shared uint sh_drawIndex;
shared uint sh_requestedDrawIndices;

#define PARTICLES_PER_THREAD 1

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
      if (pud.acceleration_A.w != 0)
      {
        pud.velocity_L.xyz += pud.acceleration_A.xyz * u_dt;
        psd.position.xyz += pud.velocity_L.xyz * u_dt;
        if (pud.velocity_L.w <= 0.0) // particle just died
        {
          pud.acceleration_A.w = 0.0;

          needFreeIndex = true;
          atomicAdd(sh_requestedFreeIndices, 1);
        }
        else // particle is alive, so we will render it (add its index to drawIndices)
        {
          needDrawIndex = true;
          atomicAdd(sh_requestedDrawIndices, 1);
        }
        pud.velocity_L.w -= u_dt;
      }
      particlesShared[index] = psd;
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