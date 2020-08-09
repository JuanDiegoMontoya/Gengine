#version 450 core

uniform sampler2DArray u_textures;
uniform int u_texIdx;

in vec2 vTexCoord;

out vec4 fragColor;

void main()
{
  fragColor = texture(u_textures, vec3(vTexCoord, u_texIdx));
}