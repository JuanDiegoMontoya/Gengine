#version 460 core
#include "../common.h"
#define KERNEL_SIZE 5

layout(binding = 0) uniform sampler2D s_specularIrradiance;
layout(binding = 1) uniform sampler2D s_gBufferDepth;
layout(binding = 2) uniform sampler2D s_gBufferNormal;
layout(binding = 3) uniform sampler2D s_gBufferPBR;
layout(binding = 0) uniform writeonly image2D i_target;

layout(location = 0) uniform float u_nPhi;
layout(location = 1) uniform float u_pPhi;
layout(location = 2) uniform float u_stepwidth;
layout(location = 3) uniform mat4 u_invViewProj;
layout(location = 4) uniform bool u_horizontal;
layout(location = 5) uniform ivec2 u_targetDim;
layout(location = 6) uniform float u_kernel[KERNEL_SIZE];
layout(location = 11) uniform float u_offsets[KERNEL_SIZE];

layout(local_size_x = 16, local_size_y = 16) in;
void main()
{
  ivec2 gid = ivec2(gl_GlobalInvocationID.xy);
  if (any(greaterThanEqual(gid, u_targetDim)))
    return;
  vec2 uv = (vec2(gid) + 0.5) / u_targetDim;

  vec2 texel = 1.0 / u_targetDim;

  vec3 pbr = textureLod(s_gBufferPBR, uv, 0).xyz;
  float roughness = pbr.x;
  if (roughness >= 0.99)
  {
    imageStore(i_target, gid, vec4(0, 0, 0, 1));
    return;
  }

  vec4 cval = textureLod(s_specularIrradiance, uv, 0);

  if (roughness <= 0.03)
  {
    imageStore(i_target, gid, cval);
    return;
  }

  vec4 nval = vec4(textureLod(s_gBufferNormal, uv, 0).xyz, 0.0);
  vec4 pval = vec4(UnprojectUV(textureLod(s_gBufferDepth, uv, 0).r, uv, u_invViewProj), 1.0);

  vec4 sum = vec4(0.0);
  float cum_w = 0.0;
  for (int i = 0; i < KERNEL_SIZE; i++)
  {
    vec2 offset = u_horizontal ? vec2(u_offsets[i], 0) : vec2(0, u_offsets[i]);
    vec2 newUV = uv + offset * texel * u_stepwidth;

    if (textureLod(s_gBufferPBR, newUV, 0).x >= 0.99)
    {
      continue;
    }

    vec4 ctmp = textureLod(s_specularIrradiance, newUV, 0);

    vec4 ntmp = vec4(textureLod(s_gBufferNormal, newUV, 0).xyz, 0.0);
    vec4 t = nval - ntmp;
    float dist2 = max(dot(t, t) / (u_stepwidth * u_stepwidth), 0.0);
    float n_w = min(exp(-(dist2) / u_nPhi), 1.0);
    
    vec4 ptmp = vec4(UnprojectUV(textureLod(s_gBufferDepth, newUV, 0).r, newUV, u_invViewProj), 1.0);
    t = pval - ptmp;
    dist2 = dot(t, t);
    float p_w = min(exp(-(dist2) / u_pPhi), 1.0);

    float weight = n_w * p_w;
    sum += ctmp * weight * u_kernel[i];
    cum_w += weight * u_kernel[i];
  }

  imageStore(i_target, gid, sum / cum_w);
}