#version 450 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D depthMap;
uniform float near_plane = .1;
uniform float far_plane = 500.;
uniform int perspective = 0;

float LinearizeDepth(float depth)
{
  float z = depth * 2.0 - 1.0; // Back to NDC 
  return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));
}

void main()
{
  float depthValue = texture(depthMap, TexCoords).r;
  if (perspective > 0)
    FragColor = vec4(vec3(LinearizeDepth(depthValue) / far_plane), 1.0); // perspective
  else
    FragColor = vec4(vec3(depthValue), 1.0); // orthographic
}