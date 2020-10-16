#version 460 core
out vec4 color;

in vec2 TexCoord;

uniform sampler2D albedoMap;

void main()
{
  color = texture(albedoMap, TexCoord);
}