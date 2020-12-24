#version 460 core

#include "particle.h"
#include "DrawArraysCommand.h"
layout (std430, binding = 0) buffer data
{
  Particle particles[];
};

layout (std430, binding = 1) buffer stack
{
  DrawArraysCommand cmd;
  int freeCount;
  uint indices[];
};

uniform float u_dt;

void UpdateParticle(inout Particle particle, int i)
{
  if (particle.alive)
  {
    particle.life -= u_dt;
    particle.velocity += particle.accel * u_dt;
    particle.pos += particle.velocity * u_dt;
    if (particle.life < 0)
    {
      particle.color.a = 0;
      particle.alive = 0;
      //emitter.numParticles--;
      //emitter.freedIndices.push(i);
      atomicAdd(cmd.instanceCount, -1);
      int index = atomicAdd(freeCount, 1);
      indices[index] = i;
    }
  }
}

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;
void main()
{
  uint start = gl_GlobalInvocationID.x;
  uint stride = gl_NumWorkGroups.x * gl_WorkGroupSize.x;
  //uint particlesPerThread = 

  for (uint i = start; i < particles.length(); i += stride)
  {
    UpdateParticle(particles[i], i);
  }
}