#version 460 core

#include "DrawArraysCommand.h"

// normal vertex buffer containing vertices for a cube
layout(location = 0) in vec3 aPos;

layout(std430, binding = 0) readonly buffer vertexBufferData
{
  int vbo[];
};

layout(std430, binding = 1) readonly buffer cmds
{
  DrawArraysCommand drawCommands[];
};

// global info
layout(location = 0) uniform mat4 u_viewProj;
layout(location = 1) uniform uint u_chunk_size;
layout(location = 2) uniform uint u_vertexSize = 8;

layout(location = 0) out flat int vID;

void main()
{
  vID = gl_InstanceID; // index of chunk being drawn
  uint aOffset = drawCommands[vID].first * 2; // ratio between vertex size and int
  vec3 cPos = { vbo[aOffset], vbo[aOffset+1], vbo[aOffset+2] };
  //float err = distance(cPos, u_viewpos) / ...
  vec3 vPos = cPos + (aPos * 1.1 + .5) * (u_chunk_size);
  gl_Position = u_viewProj * vec4(vPos, 1.0);
}