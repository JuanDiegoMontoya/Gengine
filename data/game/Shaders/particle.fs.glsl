#version 460 core
layout (location = 0) in vec2 vTexCoord;
layout (location = 1) in vec4 vColor;

layout (binding = 0) uniform sampler2D u_sprite;

layout (location = 0) out vec4 fragColor;

void main()
{
  fragColor = (texture(u_sprite, vTexCoord) * vColor);
  // if (fragColor.a < .01)
  // {
  //   discard;
  // }
}