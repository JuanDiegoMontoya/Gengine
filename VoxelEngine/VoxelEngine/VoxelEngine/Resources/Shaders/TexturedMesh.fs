#version 460 core

uniform sampler2D albedoMap;

out vec4 color;
in vec2 TexCoord;

void main()
{
  color = texture(albedoMap, TexCoord);
}