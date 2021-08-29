#version 460 core

#include "indirect.h.glsl"

// normal vertex buffer containing vertices for a cube
//layout(location = 0) in vec3 aPos;

layout(std430, binding = 0) restrict readonly buffer vertexBufferData
{
  int vbo[];
};

layout(std430, binding = 1) restrict readonly buffer cmds
{
  DrawArraysCommand drawCommands[];
};

// global info
layout(location = 0) uniform mat4 u_viewProj;
layout(location = 1) uniform uint u_chunk_size;
layout(location = 2) uniform uint u_vertexSize = 8;

layout(location = 0) out flat int vID;

vec3 CreateCube(in uint vertexID)
{
  uint b = 1 << vertexID;
  return vec3((0x287a & b) != 0, (0x02af & b) != 0, (0x31e3 & b) != 0);
}

void main()
{
  vec3 aPos = CreateCube(gl_VertexID) - .5; // gl_VertexIndex for Vulkan

  vID = gl_InstanceID; // index of chunk being drawn
  uint aOffset = drawCommands[vID].baseInstance * 2; // ratio between vertex size and int
  vec3 cPos = { vbo[aOffset], vbo[aOffset+1], vbo[aOffset+2] };
  vec3 vPos = cPos + (aPos * 1.001 + .5) * (u_chunk_size); // add tiny constant to ensure occlusion volume exceeds that of the chunk
  gl_Position = u_viewProj * vec4(vPos, 1.0);
}