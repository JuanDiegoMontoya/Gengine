#version 460 core

#include "common.h"
#include "pbr.h"

layout(location = 0) in vec2 vTexCoord;

layout(binding = 0) uniform sampler2D u_screenDepth;
layout(binding = 1) uniform sampler2D u_screenRoughnessMetalness;
layout(binding = 2) uniform samplerCube u_ReflectionCubemap;
layout(binding = 3) uniform samplerCube u_ReflectionCubemapDistance;
layout(binding = 4) uniform samplerCube u_SkyCube;
layout(binding = 5) uniform sampler2D u_blueNoise;
layout(binding = 6) uniform sampler2D u_screenDiffuse;

layout(location = 0) uniform mat4 u_invProj;
layout(location = 1) uniform mat4 u_invView;
layout(location = 2) uniform vec3 u_viewPos;

// amount to increase the step distance by after each step
// higher values allow the longer reflections at the expense of stability
uniform float stepIncreaseFactor = 1.04;
uniform float stepIncreaseConstant = 0.04;
uniform int stepIncreaseAfter = 30;
uniform float initialStepDist = 0.20;
uniform int raySteps = 60;
uniform int binarySearchIterations = 4;
uniform uint u_samples = 1;

// stopgap solution to precision issues when camera is far away
uniform float u_cameraFadeBegin = 50;
uniform float u_cameraFadeEnd = 60;

// % of max steps to begin fade
uniform float skyboxFadeBegin = 0.90;

layout(location = 0) out vec4 fragColor;





float CalcLod(uint samples, vec3 N, vec3 H, float roughness, ivec2 textureDim)
{
  float dist = D_GGX(N, H, roughness);
  return 0.5 * (log2(float(textureDim.x * textureDim.y) / samples) - log2(dist));
}

float CalcLod2(uint samples, float rayDistToHit, float zHit, float roughness, ivec2 textureDim)
{
  //float dist = 
  return 0.0;
}






vec3 CalcMissReflection(vec3 dir, float lod)
{
  return textureLod(u_SkyCube, dir, lod).rgb;
}

vec3 CalcHitReflection(vec3 dir, float lod)
{
  return textureLod(u_ReflectionCubemap, dir, lod).rgb;
}

//https://atyuwen.github.io/posts/normal-reconstruction/#fn:1
// vec3 GetNormalPrecise(vec3 pos) {}

vec3 GetNormal(vec3 pos)
{
  return normalize(cross(dFdxFine(pos), dFdyFine(pos)));
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
  //return CalcHitReflection(normalize(rayPos - u_viewPos), lod);
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
    //vec2 Xi1 = Hammersley(i, samples);
    ivec2 texel = ivec2(gl_FragCoord.xy) % ivec2(textureSize(u_blueNoise, 0));
    vec2 Xi = min(texelFetch(u_blueNoise, texel, 0).xy, vec2(0.99));
    //vec2 Xi = (Xi1 + Xi2) / 2.0;
    vec3 H = ImportanceSampleGGX(Xi, N, roughness);
    vec3 L = normalize(reflect(V, H));
    vec3 mirrorL = reflect(V, N);
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
  vec2 pbr = texture(u_screenRoughnessMetalness, vTexCoord).xy;
  float roughness = pbr.x;
  float metalness = pbr.y;
  float gBufferDepth = texture(u_screenDepth, vTexCoord).x;

  if (gBufferDepth == 0.0 || gBufferDepth == 1.0)
    discard;
  if (roughness > .99)
    discard;

  // temp
  roughness = 0.10;
  metalness = 1.0;

  // reconstruct position in view space rather than world space
  // this is important for ensuring partial derivatives have maximum precision
  vec3 gBufferViewPos = WorldPosFromDepthUV(gBufferDepth, vTexCoord, u_invProj);
  vec3 gBufferWorldPos = (u_invView * vec4(gBufferViewPos, 1.0)).xyz;
  vec3 gBufferWorldNormal = GetNormal(gBufferWorldPos - u_viewPos);
  
  vec3 V = normalize(gBufferWorldPos - u_viewPos);
  vec3 L = reflect(V, gBufferWorldNormal);

  //vec3 trace = TraceCubemap(gBufferWorldPos, gBufferWorldNormal, V, L, 0);
  //fragColor = vec4(trace, 0.9);

  vec3 gBufferDiffuse = texture(u_screenDiffuse, vTexCoord).rgb;

  // temp
  gBufferDiffuse = vec3(1);

  // not physical at all because gBufferDiffuse already has diffuse lighting applied to it
  vec3 F0 = mix(vec3(0.04), gBufferDiffuse, metalness);
  vec3 envSpecular = ComputeSpecularRadiance(gBufferWorldPos, gBufferWorldNormal, V, F0, roughness);
  fragColor = vec4(envSpecular, .5);
}

// TODO
// improved normals
// consider lighting model