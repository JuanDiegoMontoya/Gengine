#version 460 core

// normal vertex buffer containing vertices for a cube
layout (location = 0) in vec3 aPos;

struct DrawArraysCommand
{
  uint count;
  uint instanceCount;
  uint first;
  uint baseInstance;
};

layout (std430, binding = 0) readonly buffer vertexBufferData
{
  int vbo[];
};

layout(std430, binding = 1) readonly buffer cmds
{
  DrawArraysCommand drawCommands[];
};

// global info
uniform mat4 u_viewProj;
uniform uint u_chunk_size;
uniform uint u_vertexSize = 8;

out vec3 vPos;
out flat int vID;

void main()
{
  vID = gl_InstanceID; // index of chunk being drawn
  uint aOffset = drawCommands[vID].first * 2; // ratio between vertex size and int
  vec3 cPos = { vbo[aOffset], vbo[aOffset+1], vbo[aOffset+2] };
  //float err = distance(cPos, u_viewpos) / ...
  vPos = cPos + (aPos * 1.1 + .5) * (u_chunk_size);
  gl_Position = u_viewProj * vec4(vPos, 1.0);
}