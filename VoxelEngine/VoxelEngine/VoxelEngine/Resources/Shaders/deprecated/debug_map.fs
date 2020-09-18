#version 450 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D map;

void main()
{
  vec3 value = texture2D(map, TexCoords).rgb;
  FragColor = vec4(value, 1.0); // orthographic
}