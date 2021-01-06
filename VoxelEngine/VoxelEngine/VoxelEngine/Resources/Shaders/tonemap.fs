#version 460 core

layout (location = 0) uniform float u_exposure = 1.0;
layout (location = 1) uniform sampler2D u_hdrBuffer;
layout (location = 2) uniform bool u_gammaCorrection = false;

layout (location = 0) in vec2 vTexCoord;
layout (location = 0) out vec4 fragColor;

void main()
{
  const vec3 avgColor = textureLod(u_hdrBuffer, vTexCoord, textureQueryLevels(u_hdrBuffer) - 1).rgb;
  float luminance = pow((avgColor.r * 0.3) + (avgColor.g * 0.59) + (avgColor.b * 0.11), 1);
  const float gamma = 2.2;
  vec3 hdrColor = texture(u_hdrBuffer, vTexCoord).rgb;
  
  // exposure tone mapping
  vec3 mapped = hdrColor;
  //float exposure = u_exposure;
  float exposure = .99 / luminance;
  mapped = vec3(1.0) - exp(-hdrColor * exposure);
  
  // gamma correction 
  if (u_gammaCorrection == true)
  {
    mapped = pow(mapped, vec3(1.0 / gamma));
  }

  //fragColor = vec4(mapped * .001 + vec3(vTexCoord, 0.0), 1.0);
  fragColor = vec4(mapped, 1.0);
}