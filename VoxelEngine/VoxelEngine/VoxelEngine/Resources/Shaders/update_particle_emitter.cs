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

uniform int u_particlesToSpawn;

layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;
void main()
{
  uint start = gl_GlobalInvocationID.x;
  uint stride = gl_NumWorkGroups.x * gl_WorkGroupSize.x;

  for (uint i = start; i < u_particlesToSpawn; i += stride)
  {

  }
}