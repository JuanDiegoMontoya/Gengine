#version 460 core
layout (location = 0) uniform mat4 u_proj;
layout (location = 1) uniform mat4 u_modview;

layout (location = 0) out vec3 vTexCoord;

vec3 CreateCube(in uint vertexID)
{
  uint b = 1 << vertexID;
  return vec3((0x287a & b) != 0, (0x02af & b) != 0, (0x31e3 & b) != 0);
}

void main()
{
  vec3 aPos = CreateCube(gl_VertexID) - .5; // gl_VertexIndex for Vulkan
  vTexCoord = aPos;
  gl_Position = u_proj * u_modview * vec4(aPos, 1.0);
}