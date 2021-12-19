#version 460 core
#include "../common.h"
#include "pbr.h"

layout(local_size_x = 16,local_size_y = 16) in;

#define ROUGHNESS_DISCARD_THRESHOLD 0.99

layout(binding = 0) uniform sampler2D s_gBufferDepth;
layout(binding = 1) uniform sampler2D s_gBufferPBR;
layout(binding = 2) uniform sampler2D s_gBufferNormal;
layout(binding = 3) uniform sampler2D s_gBufferDiffuse;
layout(binding = 4) uniform sampler2D s_specularIrradiance;
layout(binding = 0) uniform restrict writeonly image2D i_target;

layout(location = 0) uniform mat4 u_invProj;
layout(location = 1) uniform mat4 u_invView;
layout(location = 2) uniform vec3 u_viewPos;
layout(location = 3) uniform ivec2 u_targetDim;

// interpolates up to 4 of neighbor texels' specular irradiance
vec3 fixupSpecularIrradiance(vec2 uv)
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
    vec2 newUV = uv + offsets[i] / vec2(textureSize(s_specularIrradiance, 0));
    vec3 neighborSample = textureLod(s_specularIrradiance, newUV, 0).rgb;
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
  ivec2 gid = ivec2(gl_GlobalInvocationID.xy);
  if (any(greaterThanEqual(gid, u_targetDim)))
    return;
  vec2 uv = (vec2(gid) + 0.5) / u_targetDim;

  vec3 pbr = textureLod(s_gBufferPBR, uv, 0).xyz;
  float roughness = pbr.x;
  float metalness = pbr.y;
  float diffuseBlend = pbr.z;
  float gBufferDepth = textureLod(s_gBufferDepth, uv, 0).x;

  vec3 gBufferDiffuse = textureLod(s_gBufferDiffuse, uv, 0).rgb;
  vec3 specularIrradiance = textureLod(s_specularIrradiance, uv, 0).rgb;

  // we have a fragment that should receive a reflected color, but isn't because the irradiance map is low res
  // attempt fixup by interpolating neighbors
  if (specularIrradiance == vec3(0) && roughness <= ROUGHNESS_DISCARD_THRESHOLD && 
      textureSize(s_gBufferPBR, 0) != textureSize(s_specularIrradiance, 0))
  {
    specularIrradiance = fixupSpecularIrradiance(uv);
  }

  if (gBufferDepth == 0.0 || gBufferDepth == 1.0 || roughness > ROUGHNESS_DISCARD_THRESHOLD || specularIrradiance == vec3(0))
  {
    imageStore(i_target, gid, gBufferDiffuse.rgbb);
    return;
  }

  // reconstruct position in view space rather than world space
  // this is important for ensuring partial derivatives have maximum precision
  vec3 gBufferViewPos = UnprojectUV(gBufferDepth, uv, u_invProj);
  vec3 gBufferWorldPos = (u_invView * vec4(gBufferViewPos, 1.0)).xyz;
  

  vec3 N = textureLod(s_gBufferNormal, uv, 0).xyz;

  vec3 V = normalize(gBufferWorldPos - u_viewPos);
  vec3 L = reflect(V, N);

  vec3 F0 = mix(vec3(0.04), gBufferDiffuse, metalness);
  float NoV = max(dot(N, V), 0.0);
  vec3 kS = fresnelSchlickRoughness(NoV, F0, roughness);
  vec3 kD = 1.0 - kS;
  kD *= 1.0 - metalness;

  // not physical at all because gBufferDiffuse already has diffuse lighting applied to it
  vec3 shaded = gBufferDiffuse * kD + specularIrradiance;
  imageStore(i_target, gid, shaded.rgbb);
  //o_shaded.rgb = o_shaded.rgb * .00001 + specularIrradiance;
}