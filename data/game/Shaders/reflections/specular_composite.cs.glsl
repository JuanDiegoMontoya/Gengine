#version 460 core
#include "../common.h"
#include "pbr.h"

layout(local_size_x = 16,local_size_y = 16) in;

#define ROUGHNESS_DISCARD_THRESHOLD 0.99

layout(binding = 0) uniform sampler2D s_gBufferDepth;
layout(binding = 1) uniform sampler2D s_gBufferDiffuse;
layout(binding = 2) uniform sampler2D s_gBufferNormal;
layout(binding = 3) uniform sampler2D s_gBufferPBR;
layout(binding = 4) uniform sampler2D s_specularIrradiance;
layout(binding = 0) uniform restrict writeonly image2D i_target;

layout(location = 0) uniform mat4 u_invProj;
layout(location = 1) uniform mat4 u_invView;
layout(location = 2) uniform vec3 u_viewPos;
layout(location = 3) uniform ivec2 u_sourceDim;
layout(location = 4) uniform ivec2 u_targetDim;

// interpolates up to 4 of neighbor texels' specular irradiance
vec3 fixupSpecularIrradiance(ivec2 gid)
{
  const ivec2 offsets[4] = {
    { -1, 0 },
    { 0, 1 },
    { 1, 0 },
    { 0, -1 }
  };

  vec3 accumColor = vec3(0);
  float weight = 0;
  for (int i = 0; i < 4; i++)
  {
    vec3 neighborSample = texelFetch(s_specularIrradiance, gid + offsets[i], 0).rgb;
    if (neighborSample != vec3(0))
    {
      accumColor += neighborSample;
      weight += 1.0;
    }
  }

  if (weight > 0)
    return accumColor / weight;
  return vec3(0.0);
}

void main()
{
  ivec2 gid = ivec2(gl_GlobalInvocationID.xy);
  if (any(greaterThanEqual(gid, u_targetDim)))
    return;
  vec2 uv = (vec2(gid) + 0.5) / u_targetDim;
  vec2 texel = 1.0 / u_sourceDim;
  vec2 ratio = vec2(u_targetDim) / vec2(u_sourceDim);

  vec3 pbr = texelFetch(s_gBufferPBR, gid, 0).xyz;
  float roughness = pbr.x;
  float metalness = pbr.y;
  float diffuseBlend = pbr.z;
  float gBufferDepth = texelFetch(s_gBufferDepth, gid, 0).x;

  vec3 gBufferDiffuse = texelFetch(s_gBufferDiffuse, gid, 0).rgb;
  vec3 specularIrradiance = texelFetch(s_specularIrradiance, ivec2(gid / ratio), 0).rgb;

  if (specularIrradiance == vec3(0) && roughness <= ROUGHNESS_DISCARD_THRESHOLD)
  {
    //imageStore(i_target, gid, vec4(1, 0, 0, 0));
    //return;
    specularIrradiance = fixupSpecularIrradiance(ivec2(gid / ratio));
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
  
  vec3 N = texelFetch(s_gBufferNormal, gid, 0).xyz;

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