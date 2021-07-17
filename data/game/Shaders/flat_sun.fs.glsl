#version 450 core

uniform vec4 u_color;

out vec4 color;

void main()
{
  color = u_color;
}