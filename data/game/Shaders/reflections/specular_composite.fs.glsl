#version 460 core

#include "../common.h"
#include "pbr.h"

#define ROUGHNESS_DISCARD_THRESHOLD 0.99

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

// interpolates up to 4 of neighbor texels' specular irradiance
vec3 fixupSpecularIrradiance()
{
  const vec2 offsets[4] = {
    { -1, 0 },
    { 0, 1 },
    { 1, 0 },
    { 0, -1 }
  };

  vec3 accumColor = vec3(0);
  float weight = 0;
  for (int i = 0; i < 4; i++)
  {
    vec2 newUV = vTexCoord + offsets[i] / vec2(textureSize(u_specularIrradianceTex, 0));
    vec3 neighborSample = texture(u_specularIrradianceTex, newUV).rgb;
    if (neighborSample != vec3(0))
    {
      accumColor += neighborSample;
      weight += 1.0;
    }
  }

  if (weight > 0)
    return accumColor / weight;
  return vec3(1.0);
}

void main()
{
  vec3 pbr = texture(u_gBufferPBR, vTexCoord).xyz;
  float roughness = pbr.x;
  float metalness = pbr.y;
  float diffuseBlend = pbr.z;
  float gBufferDepth = texture(u_gBufferDepth, vTexCoord).x;

  vec3 gBufferDiffuse = texture(u_gBufferDiffuse, vTexCoord).rgb;
  vec3 specularIrradiance = texture(u_specularIrradianceTex, vTexCoord).rgb;

  // we have a fragment that should receive a reflected color, but isn't because the irradiance map is low res
  // attempt fixup by interpolating neighbors
  if (specularIrradiance == vec3(0) && roughness <= ROUGHNESS_DISCARD_THRESHOLD && 
      textureSize(u_gBufferPBR, 0) != textureSize(u_specularIrradianceTex, 0))
  {
    specularIrradiance = fixupSpecularIrradiance();
  }

  if (gBufferDepth == 0.0 || gBufferDepth == 1.0 || roughness > ROUGHNESS_DISCARD_THRESHOLD || specularIrradiance == vec3(0))
  {
    o_shaded = vec4(gBufferDiffuse, 1.0);
    return;
  }

  // reconstruct position in view space rather than world space
  // this is important for ensuring partial derivatives have maximum precision
  vec3 gBufferViewPos = UnprojectUV(gBufferDepth, vTexCoord, u_invProj);
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