#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 2) in vec2 aTexCoord;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_proj;

out vec2 vTexCoord;

void main()
{
  vTexCoord = aTexCoord;
  gl_Position = u_proj * u_view * u_model * vec4(aPos.xyz, 1.0f);
}
