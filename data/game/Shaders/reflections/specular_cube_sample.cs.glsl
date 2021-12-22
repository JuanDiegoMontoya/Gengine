#version 460 core
#include "../common.h"
#include "pbr.h"

layout(local_size_x = 16, local_size_y = 16) in;

layout(binding = 0) uniform sampler2D s_gBufferDepth;
layout(binding = 1) uniform sampler2D s_gBufferDiffuse;
layout(binding = 2) uniform sampler2D s_gBufferNormal;
layout(binding = 3) uniform sampler2D s_gBufferPBR;
layout(binding = 4) uniform samplerCube s_env;
layout(binding = 5) uniform sampler2D s_blueNoise;

layout(location = 0) uniform mat4 u_invProj;
layout(location = 1) uniform mat4 u_invView;
layout(location = 2) uniform vec3 u_viewPos;
layout(location = 3) uniform ivec2 u_targetDim;

layout(binding = 0) uniform writeonly restrict image2D i_target;

vec3 ComputeSpecularIrradianceSample(vec3 N, vec3 V, vec3 F0, float roughness)
{
  ivec2 gid = ivec2(gl_GlobalInvocationID.xy);
  float NoV = abs(dot(N, V));

  vec3 accumColor = vec3(0.0);
  
  ivec2 texel = gid % ivec2(textureSize(s_blueNoise, 0));
  vec2 Xi = min(texelFetch(s_blueNoise, texel, 0).xy, vec2(0.99));
  vec3 H = ImportanceSampleGGX(Xi, N, roughness);
  vec3 L;
  if (roughness > 0.03)
    L = normalize(reflect(V, H));
  else
    L = reflect(V, N);

  float NoH = abs(dot(N, H));
  float VoH = abs(dot(V, H));
  vec3 F = fresnelSchlick(VoH, F0);

  vec3 envColor = textureLod(s_env, L, 0).rgb;

  accumColor += F * envColor * VoH / (NoH * NoV);

  return accumColor;
}

void main()
{
  ivec2 gid = ivec2(gl_GlobalInvocationID.xy);
  if (any(greaterThanEqual(gid, u_targetDim)))
    return;
  vec2 uv = (vec2(gid) + 0.5) / u_targetDim;

  vec2 pbr = textureLod(s_gBufferPBR, uv, 0).xy;
  float roughness = pbr.x;
  float metalness = pbr.y;
  float gBufferDepth = textureLod(s_gBufferDepth, uv, 0).x;

  if (gBufferDepth == 0.0 || gBufferDepth == 1.0 || roughness > .99)
  {
    imageStore(i_target, gid, vec4(0.0));
    return;
  }

  // reconstruct position in view space rather than world space
  // this is important for ensuring partial derivatives have maximum precision
  vec3 gBufferViewPos = UnprojectUV(gBufferDepth, uv, u_invProj);
  vec3 gBufferWorldPos = (u_invView * vec4(gBufferViewPos, 1.0)).xyz;
  
  vec3 diffuse = textureLod(s_gBufferDiffuse, uv, 0).rgb;

  vec3 N = textureLod(s_gBufferNormal, uv, 0).xyz;
  vec3 V = normalize(gBufferWorldPos - u_viewPos);
  vec3 F0 = mix(vec3(0.04), diffuse, metalness);

  vec3 color = ComputeSpecularIrradianceSample(N, V, F0, roughness);

  imageStore(i_target, gid, vec4(color, 0.0));
}