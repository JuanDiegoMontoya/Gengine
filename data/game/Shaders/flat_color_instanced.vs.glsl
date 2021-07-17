#version 450 core

layout (location = 0) in vec3 aScreenPos;
layout (location = 1) in vec4 aColor;
layout (location = 2) in mat4 aModel;
//layout (location = 1) in mat4 aModel;

uniform mat4 u_viewProj; // p * v
out vec4 color;

void main()
{
  color = aColor;
  gl_Position = u_viewProj * aModel * vec4(aScreenPos.xyz, 1.0f);
}
