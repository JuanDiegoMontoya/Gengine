#version 450 core

layout(location = 0) in vec3 vColor;

layout(location = 0) out vec4 fragColor;

void main()
{
  gl_FragDepth = 1;
  fragColor = vec4(vColor, 1.0);
}