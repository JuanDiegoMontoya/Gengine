#version 460 core

#include "../common.h"
#include "pbr.h"

layout(location = 0) in vec2 vTexCoord;

layout(binding = 0) uniform sampler2D u_gBufferDepth;
layout(binding = 1) uniform sampler2D u_gBufferPBR;
layout(binding = 2) uniform samplerCube u_ReflectionCubemapDiffuse;
layout(binding = 3) uniform samplerCube u_ReflectionCubemapDistance;
layout(binding = 4) uniform samplerCube u_SkyCube;
layout(binding = 5) uniform sampler2D u_blueNoise;
layout(binding = 6) uniform sampler2D u_gBufferNormal;
layout(binding = 7) uniform sampler2D u_gBufferDiffuse;

layout(location = 0) uniform mat4 u_invProj;
layout(location = 1) uniform mat4 u_invView;
layout(location = 2) uniform vec3 u_viewPos;

// amount to increase the step distance by after each step
// higher values allow the longer reflections at the expense of stability
layout(location = 3) uniform float stepIncreaseFactor = 1.04;
layout(location = 4) uniform float stepIncreaseConstant = 0.04;
layout(location = 5) uniform int stepIncreaseAfter = 30;
layout(location = 6) uniform float initialStepDist = 0.20;
layout(location = 7) uniform int raySteps = 60;
layout(location = 8) uniform int binarySearchIterations = 4;
layout(location = 9) uniform uint u_samples = 1;

// stopgap solution for precision issues when camera is far away
layout(location = 10) uniform float u_cameraFadeBegin = 50;
layout(location = 11) uniform float u_cameraFadeEnd = 60;

// % of max steps to begin fade
layout(location = 12) uniform float skyboxFadeBegin = 0.90;

layout(location = 0) out vec4 o_specularIrradiance;





float CalcLod(uint samples, vec3 N, vec3 H, float roughness, ivec2 textureDim)
{
  float dist = D_GGX(N, H, roughness);
  return 0.5 * (log2(float(textureDim.x * textureDim.y) / samples) - log2(dist));
}





vec3 CalcMissReflection(vec3 dir, float lod)
{
  return textureLod(u_SkyCube, dir, lod).rgb;
}

vec3 CalcHitReflection(vec3 dir, float lod)
{
  return textureLod(u_ReflectionCubemapDiffuse, dir, lod).rgb;
}

vec3 BinarySearch(vec3 rayPos, vec3 reflectDir, float stepDist, float maxError)
{
  for (int i = 0; i < binarySearchIterations; i++) 
  {
    vec3 sampleDir = normalize(rayPos - u_viewPos);
    float z = textureLod(u_ReflectionCubemapDistance, sampleDir, 0).x;
    float error = distance(z, distance(rayPos, u_viewPos));

    stepDist /= 2.0;
    rayPos += reflectDir * stepDist * (error < maxError ? -1.0 : 1.0);
  }

  return normalize(rayPos - u_viewPos);
}

float TraceCubemap(vec3 rayStart, vec3 N, vec3 V, vec3 reflectDir, out vec3 cubeHit)
{
  float stepDist = initialStepDist;
  float maxError = stepDist;

  // random jitter along ray step to reduce quantization artifacts
  ivec2 texel = ivec2(gl_FragCoord.xy) % ivec2(textureSize(u_blueNoise, 0));
  float jitter = texelFetch(u_blueNoise, texel, 0).x;

  // amount to offset the ray along the surface normal to prevent self-intersection
  // rays perpendicular to the normal are pushed the most
  float angleBias = max(mix(0, maxError, 2.0 * dot(-V, N)), maxError);
  vec3 rayPos = (rayStart + N * angleBias) + reflectDir * stepDist * jitter;
  
  for (int i = 0; i < raySteps; i++)
  {
    vec3 sampleDir = normalize(rayPos - u_viewPos);
    float z = textureLod(u_ReflectionCubemapDistance, sampleDir, 0).x;
    float error = distance(z, distance(rayPos, u_viewPos));

    if (error < maxError)
    {
      float blend = smoothstep(float(raySteps), float(raySteps) * skyboxFadeBegin, i);
      cubeHit = BinarySearch(rayPos, reflectDir, stepDist, maxError);
      return blend;
    }

    // Crytek paper suggests a per-step jitter to reduce staircase artifacts
    // I'm aware that I'm reusing the original jitter, but it seems to work just fine
    rayPos += reflectDir * stepDist * (0.95 + 0.1 * jitter);
    if (i > stepIncreaseAfter)
    {
      stepDist *= stepIncreaseFactor;
      stepDist += stepIncreaseConstant;
      maxError = stepDist;
    }
  }

  // miss
  return 0.0;
}

vec3 ComputeSpecularRadiance(vec3 rayStart, vec3 N, vec3 V, vec3 F0, float roughness)
{
  float NoV = abs(dot(N, V));

  vec3 accumColor = vec3(0.0);
  
  const uint samples = u_samples;
  for (uint i = 0; i < samples; i++)
  {
    ivec2 texel = ivec2(gl_FragCoord.xy) % ivec2(textureSize(u_blueNoise, 0));
    vec2 Xi = min(texelFetch(u_blueNoise, texel, 0).xy, vec2(0.99));
    vec3 H = ImportanceSampleGGX(Xi, N, roughness);
    vec3 L;
    if (roughness > 0.03)
      L = normalize(reflect(V, H));
    else
      L = reflect(V, N);
    float lod = CalcLod(samples, N, H, roughness, textureSize(u_SkyCube, 0));
    //float lod = 0;

    float NoH = abs(dot(N, H));
    float VoH = abs(dot(V, H));
    vec3 F = fresnelSchlick(VoH, F0);

    vec3 cubeHit;
    float cameraBlend = smoothstep(u_cameraFadeEnd, u_cameraFadeBegin, distance(rayStart, u_viewPos));
    float rayBlend = TraceCubemap(rayStart, N, V, L, cubeHit);
    vec3 hitSample = vec3(0.0);
    if (rayBlend > 0.01)
    {
      hitSample = CalcHitReflection(cubeHit, 0);
    }
    vec3 missSample = CalcMissReflection(L, 0);
    float blend = cameraBlend * rayBlend;
    vec3 lColor = mix(missSample, hitSample, blend);

    accumColor += F * lColor * VoH / (NoH * NoV);
    //accumColor += lColor;
  }

  return accumColor / float(samples);
}

void main()
{
  vec2 pbr = textureLod(u_gBufferPBR, vTexCoord, 0).xy;
  float roughness = pbr.x;
  float metalness = pbr.y;
  float gBufferDepth = textureLod(u_gBufferDepth, vTexCoord, 0).x;

  if (gBufferDepth == 0.0 || gBufferDepth == 1.0 || roughness > .99)
  {
    o_specularIrradiance = vec4(0, 0, 0, 1);
    return;
  }

  // reconstruct position in view space rather than world space
  // this is important for ensuring partial derivatives have maximum precision
  vec3 gBufferViewPos = WorldPosFromDepthUV(gBufferDepth, vTexCoord, u_invProj);
  vec3 gBufferWorldPos = (u_invView * vec4(gBufferViewPos, 1.0)).xyz;
  
  vec3 diffuse = textureLod(u_gBufferDiffuse, vTexCoord, 0).rgb;

  vec3 N = textureLod(u_gBufferNormal, vTexCoord, 0).xyz;
  //vec3 vpos = gBufferWorldPos - u_viewPos;
  //vec3 N = normalize(cross(dFdxFine(vpos), dFdyFine(vpos)));
  vec3 V = normalize(gBufferWorldPos - u_viewPos);

  vec3 F0 = mix(vec3(0.04), diffuse, metalness);

  o_specularIrradiance.rgb = ComputeSpecularRadiance(gBufferWorldPos, N, V, F0, min(roughness, 0.10));
  //o_specularIrradiance.rgb = o_specularIrradiance.rgb * .00001 + (N * .5 + .5);
  o_specularIrradiance.a = 1.0;
}