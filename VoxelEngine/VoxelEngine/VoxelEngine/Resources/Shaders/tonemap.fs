#version 460 core

layout (location = 0) uniform float u_exposureFactor = 1.0;
layout (location = 1) uniform sampler2D u_hdrBuffer;
layout (location = 2) uniform bool u_gammaCorrection = false;
layout (location = 3) uniform float u_targetLuminance = 0.5;
layout (location = 4) uniform float u_dt;
layout (location = 5) uniform float u_adjustmentSpeed;

layout (location = 0) in vec2 vTexCoord;
layout (location = 0) out vec4 fragColor;

layout (std430, binding = 0) readonly buffer exposureA
{
  float readExposure;
};

layout (std430, binding = 1) writeonly buffer exposureB
{
  float writeExposure;
};

// uses eye weights
float ColorToLuminance(in vec3 color)
{
  return (color.r * 0.3) + (color.g * 0.59) + (color.b * 0.11);
}

float GetAvgLuminanceMipTex(in sampler2D tex)
{
  vec3 avgColor = textureLod(tex, vTexCoord, textureQueryLevels(tex) - 1).rgb;
  return ColorToLuminance(avgColor);
}

void main()
{
  float luminance = GetAvgLuminanceMipTex(u_hdrBuffer);
  const float gamma = 2.2;
  vec3 hdrColor = texture(u_hdrBuffer, vTexCoord).rgb;
  
  // exposure tone mapping
  vec3 mapped = hdrColor;
  float exposureTarget = clamp(u_targetLuminance / luminance, .3, 3.0);
  float exposure = mix(readExposure, exposureTarget, u_dt * u_adjustmentSpeed);
  writeExposure = exposure;
  mapped = vec3(1.0) - exp(-hdrColor * u_exposureFactor * exposure);
  
  // gamma correction 
  if (u_gammaCorrection == true)
  {
    mapped = pow(mapped, vec3(1.0 / gamma));
  }

  //fragColor = vec4(mapped * .001 + vec3(vTexCoord, 0.0), 1.0);
  fragColor = vec4(mapped, 1.0);
}