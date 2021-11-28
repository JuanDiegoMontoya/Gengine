#version 450 core

layout(location = 3) uniform sampler2DArray u_textures;
layout(location = 4) uniform int u_texIdx;

layout(location = 0) in vec2 vTexCoord;

layout(location = 0) out vec4 fragColor;

void main()
{
  fragColor = texture(u_textures, vec3(vTexCoord, u_texIdx));
}