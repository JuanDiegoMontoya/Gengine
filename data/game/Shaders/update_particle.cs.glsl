#version 460 core
#include "particle.h"
#include "DrawArraysCommand.h"

layout (std430, binding = 0) buffer data
{
  Particle particles[];
};

layout (std430, binding = 1) buffer stack
{
  coherent int freeCount;
  int indices[];
};

layout (std430, binding = 2) coherent buffer indirectCommand
{
  DrawArraysCommand cmd;
};

layout (std430, binding = 3) buffer drawindices
{
  uint drawIndices[];
};

layout (location = 0) uniform float u_dt;

void UpdateParticle(inout Particle particle, int i)
{
  if (particle.alive != 0)
  {
    particle.velocity += particle.accel * u_dt;
    particle.pos += particle.velocity * u_dt;
    if (particle.life <= 0.0)
    {
      particle.color.a = 0.0;
      particle.alive = 0;
      int index = atomicAdd(freeCount, 1);
      indices[index] = i;
    }
    else // particle is alive, so we will render it (add its index to drawIndices)
    {
      uint indexIndex = atomicAdd(cmd.instanceCount, 1);
      drawIndices[indexIndex] = i;
    }
    particle.life -= u_dt;
  }
}

layout(local_size_x = 128, local_size_y = 1, local_size_z = 1) in;
void main()
{
  uint start = gl_GlobalInvocationID.x;
  uint stride = gl_NumWorkGroups.x * gl_WorkGroupSize.x;

  for (int i = int(start); i < particles.length(); i += int(stride))
  {
    UpdateParticle(particles[i], int(i));
  }
}