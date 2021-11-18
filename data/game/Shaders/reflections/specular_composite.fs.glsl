#version 460 core

#include "../common.h"
#include "pbr.h"

layout(location = 0) in vec2 vTexCoord;

layout(binding = 0) uniform sampler2D u_gBufferDepth;
layout(binding = 1) uniform sampler2D u_gBufferPBR;
layout(binding = 2) uniform sampler2D u_gBufferNormal;
layout(binding = 3) uniform sampler2D u_gBufferDiffuse;
layout(binding = 4) uniform sampler2D u_specularIrradianceTex;

layout(location = 0) uniform mat4 u_invProj;
layout(location = 1) uniform mat4 u_invView;
layout(location = 2) uniform vec3 u_viewPos;

layout(location = 0) out vec4 o_shaded;

void main()
{
  vec2 pbr = texture(u_gBufferPBR, vTexCoord).xy;
  float roughness = pbr.x;
  float metalness = pbr.y;
  float gBufferDepth = texture(u_gBufferDepth, vTexCoord).x;

  vec3 gBufferDiffuse = texture(u_gBufferDiffuse, vTexCoord).rgb;
  vec3 specularIrradiance = texture(u_specularIrradianceTex, vTexCoord).rgb;

  if (gBufferDepth == 0.0 || gBufferDepth == 1.0 || roughness > .99 || specularIrradiance == vec3(0))
  {
    o_shaded = vec4(gBufferDiffuse, 1.0);
    return;
  }

  // reconstruct position in view space rather than world space
  // this is important for ensuring partial derivatives have maximum precision
  vec3 gBufferViewPos = WorldPosFromDepthUV(gBufferDepth, vTexCoord, u_invProj);
  vec3 gBufferWorldPos = (u_invView * vec4(gBufferViewPos, 1.0)).xyz;
  

  vec3 N = texture(u_gBufferNormal, vTexCoord).xyz;

  vec3 V = normalize(gBufferWorldPos - u_viewPos);
  vec3 L = reflect(V, N);

  vec3 F0 = mix(vec3(0.04), gBufferDiffuse, metalness);
  float NoV = max(dot(N, V), 0.0);
  vec3 kS = fresnelSchlickRoughness(NoV, F0, roughness);
  vec3 kD = 1.0 - kS;
  kD *= 1.0 - metalness;

  // not physical at all because gBufferDiffuse already has diffuse lighting applied to it
  vec3 shaded = gBufferDiffuse * kD + specularIrradiance;
  o_shaded = vec4(shaded, 1.0);
  //o_shaded.rgb = o_shaded.rgb * .00001 + specularIrradiance;
}