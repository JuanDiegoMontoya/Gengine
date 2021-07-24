#version 460 core
#include "particle.h"
#include "indirect.h.glsl"

layout(std430, binding = 0) buffer Particles
{
  Particle particles[];
};

layout(std430, binding = 1) buffer Stack
{
  coherent int freeCount;
  int indices[];
};

layout(std430, binding = 2) coherent buffer IndirectCommand
{
  DrawArraysCommand indirectCommand;
};

layout(std430, binding = 3) buffer Drawindices
{
  uint drawIndices[];
};

layout(location = 0) uniform float u_dt;

shared int sh_freeIndex;
shared int sh_requestedFreeIndices;
shared uint sh_drawIndex;
shared uint sh_requestedDrawIndices;

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;
void main()
{
  // strategy: record how many elements we actually need, request it at once, then write to buffers

  if (gl_LocalInvocationIndex == 0)
  {
    sh_requestedFreeIndices = 0;
    sh_requestedDrawIndices = 0;
  }

  barrier();
  memoryBarrierShared();

  int index = int(gl_GlobalInvocationID.x);
  bool needFreeIndex = false;
  bool needDrawIndex = false;
  if (index < particles.length())
  {
    Particle particle = particles[index];
    if (particle.alive != 0)
    {
      particle.velocity += particle.accel * u_dt;
      particle.pos += particle.velocity * u_dt;
      if (particle.life <= 0.0) // particle just died
      {
        particle.alive = 0;

        needFreeIndex = true;
        atomicAdd(sh_requestedFreeIndices, 1);
      }
      else // particle is alive, so we will render it (add its index to drawIndices)
      {
        needDrawIndex = true;
        atomicAdd(sh_requestedDrawIndices, 1);
      }
      particle.life -= u_dt;
    }
    particles[index] = particle;
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
}