#version 460 core

layout(location = 3) uniform sampler2D albedoMap;

out vec4 color;
layout(location = 2) in vec2 TexCoord;

void main()
{
  color = texture(albedoMap, TexCoord);
}