#version 460 core

layout (std430, binding = 0) buffer exposures
{
  float readExposure;
  float writeExposure;
};

layout (std430, binding = 1) buffer out_data
{
  readonly float log_luminance;
};

layout (location = 0) uniform float u_targetLuminance = 0.5;
layout (location = 1) uniform float u_dt;
layout (location = 2) uniform float u_adjustmentSpeed;
layout (location = 3) uniform sampler2D u_hdrBuffer;
layout (location = 4) uniform float u_minExposure;
layout (location = 5) uniform float u_maxExposure;

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main()
{
  float temp = readExposure;
  readExposure = writeExposure;
  writeExposure = temp;

  //const vec3 color = textureLod(u_hdrBuffer, vec2(.5), textureQueryLevels(u_hdrBuffer) - 1).rgb;
  //const float luminance = dot(color, vec3(.3, .59, .11));
  const ivec2 texSize = textureSize(u_hdrBuffer, 0);
  const uint numPixels = texSize.x * texSize.y;
  const float luminance = exp(log_luminance / numPixels);
  
  float exposureTarget = clamp(u_targetLuminance / luminance, u_minExposure, u_maxExposure);
  writeExposure = mix(readExposure, exposureTarget, u_dt * u_adjustmentSpeed);
}