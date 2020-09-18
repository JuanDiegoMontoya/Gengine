#version 450 core

layout (location = 0) in vec3 aScreenPos;
layout (location = 1) in vec3 aColor;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_proj;

out vec4 vColor;

void main()
{
  vColor = vec4(aColor, 1.0);
  gl_Position = u_proj * u_view * u_model * vec4(aScreenPos.xyz, 1.0f);
}
