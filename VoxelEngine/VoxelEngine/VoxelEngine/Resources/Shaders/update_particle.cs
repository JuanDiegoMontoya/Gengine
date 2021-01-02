#version 460 core

#include "particle.h"
layout (std430, binding = 0) buffer data
{
  Particle particles[];
};

layout (std430, binding = 1) coherent buffer stack
{
  int freeCount;
  int indices[];
};

uniform float u_dt;

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