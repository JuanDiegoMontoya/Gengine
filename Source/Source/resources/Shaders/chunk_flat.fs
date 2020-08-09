#version 450 core

//uniform vec4 u_color;
in vec4 vColor;

out vec4 fragColor;

void main()
{
  fragColor = vColor;
}