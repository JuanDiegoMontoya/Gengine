#version 450 core

in vec3 vColor;

out vec4 fragColor;

void main()
{
  fragColor = vec4(vColor, 1.0);
}