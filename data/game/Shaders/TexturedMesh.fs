#version 460 core

layout (location = 0) in vec3 fragPos;
layout (location = 1) in vec3 Normal;
layout (location = 2) in vec2 TexCoord;

layout (location = 3, binding = 0) uniform sampler2D u_tex;

layout (location = 0) out vec4 color;

void main()
{
  vec3 texColor = texture(u_tex, TexCoord).rgb;
  vec3 normalColor = Normal * .5 + .5;
  color = vec4(mix(texColor, normalColor, fragPos / 100), 1.0);
}