#version 460 core
#include "common.h"
#define KERNEL_SIZE 5

layout (location = 0) in vec2 vTexCoord;

layout (location = 0, binding = 0) uniform sampler2D colorTex;
layout (location = 1, binding = 1) uniform sampler2D gDepth;
layout (location = 2, binding = 2) uniform sampler2D gNormal;
layout (location = 4) uniform float n_phi;
layout (location = 5) uniform float p_phi;
layout (location = 6) uniform float stepwidth;
layout (location = 7) uniform mat4 u_invViewProj;
layout (location = 8) uniform ivec2 u_resolution;
layout (location = 9) uniform bool u_horizontal;
layout (location = 10) uniform float kernel[KERNEL_SIZE];
layout (location = 23) uniform float offsets[KERNEL_SIZE];

layout (location = 0) out vec4 fragColor;

void main()
{
  vec4 sum = vec4(0.0);
  vec2 step = 1.0 / u_resolution;
  vec4 cval = texture(colorTex, vTexCoord);
  vec4 nval = vec4(texture(gNormal, vTexCoord).xyz, 0.0);
  vec4 pval = vec4(WorldPosFromDepthUV(texture(gDepth, vTexCoord).r, vTexCoord, u_invViewProj), 1.0);

  float cum_w = 0.0;
  for (int i = 0; i < KERNEL_SIZE; i++)
  {
    vec2 offset = u_horizontal ? vec2(offsets[i], 0) : vec2(0, offsets[i]);
    vec2 uv = vTexCoord + offset * step * stepwidth;

    vec4 ctmp = texture(colorTex, uv);

    vec4 ntmp = vec4(oct_to_float32x3(texture(gNormal, uv).xy), 0.0);
    vec4 t = nval - ntmp;
    float dist2 = max(dot(t, t) / (stepwidth * stepwidth), 0.0);
    float n_w = min(exp(-(dist2) / n_phi), 1.0);
    
    vec4 ptmp = vec4(WorldPosFromDepthUV(texture(gDepth, uv).r, uv, u_invViewProj), 1.0);
    t = pval - ptmp;
    dist2 = dot(t, t);
    float p_w = min(exp(-(dist2) / p_phi), 1.0);

    float weight = n_w * p_w;
    sum += ctmp * weight * kernel[i];
    cum_w += weight * kernel[i];
  }

  fragColor = sum / cum_w;
}