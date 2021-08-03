#version 460 core
#include "common.h"

#define EPSILON 1e-4

layout (location = 0) in vec2 vTexCoord;

layout(location = 0, binding = 0) uniform sampler2D u_hdrColor;
layout(location = 1, binding = 1) uniform sampler2D u_hdrDepth;
layout(location = 2) uniform mat4 u_invViewProj;
layout(location = 3) uniform ivec2 u_viewportSize;
layout(location = 4) uniform float u_distanceScale = 0.10;
layout(location = 5) uniform float u_heightOffset = -40.0;
layout(location = 6) uniform float u_hfIntensity = 1.0;
layout(location = 7) uniform vec3 u_envColor = vec3(1.0);
layout(location = 8) uniform float u_a = 0.1;
layout(location = 9) uniform float u_b = 50.0;
layout(location = 10) uniform float u_fog2Density = 50.0;

layout (location = 0) out vec4 fragColor;


void main()
{
  const vec3 hdrColor = texture(u_hdrColor, vTexCoord).xyz;
  const float depth = texture(u_hdrDepth, vTexCoord).r;
  vec3 rayEnd = WorldPosFromDepth(max(depth, .00001), u_viewportSize, u_invViewProj);
  vec3 rayStart = WorldPosFromDepth(1.0, u_viewportSize, u_invViewProj);
  vec3 rayDir = normalize(rayEnd - rayStart);
  const float totalDistance = distance(rayStart, rayEnd);

  rayStart += u_heightOffset;
  rayEnd += u_heightOffset;

  if (abs(rayDir.y) < .00001)
    rayDir.y += .0001;

  // xz-plane intercept
  float t_y = -1e7;
  if (abs(rayDir.y) > .000001)
    t_y = -rayStart.y / rayDir.y;

  vec3 rayStartA = rayStart;
  vec3 rayEndA = rayEnd;
  vec3 rayStartB = rayStart;
  vec3 rayEndB = rayEnd;
  if (rayStart.y < 0)
  {
    rayStartA = vec3(0);
    rayEndA = vec3(0);
  }
  else
  {
    rayStartB = vec3(0);
    rayEndB = vec3(0);
  }

  if (t_y > 0 && t_y < totalDistance) // cutoff in view and is in front of viewer
  {
    if (rayStart.y > 0) // viewer is above zero: cut end of rayA, start rayB there
    {
      rayStartA = rayStart;
      rayEndA = rayStart + rayDir * (t_y + EPSILON);
      rayStartB = rayEndA;
      rayEndB = rayEnd;
    }
    else // viewer is below zero: rayB begins at viewer, cut end of rayB, start rayA there
    {
      rayStartB = rayStart;
      rayEndB = rayStart + rayDir * (t_y + EPSILON);
      rayStartA = rayEndB;
      rayEndA = rayEnd;
    }
  }

  // if (t_y > 0)
  // {
  //   fragColor = vec4(1, 0, 0, 1);
  //   return;
  // }

  float scatter = 0.0;

  float fog1Top = u_a * u_b;

  float t_y2 = 1e-7;
  if (abs(rayDir.y) > .000001)
  {
    t_y2 = (fog1Top - rayStartA.y) / rayDir.y;
  }

  if ((rayDir.y > 0 && rayStart.y > fog1Top) || (rayDir.y < 0 && rayStart.y < 0) || (t_y2 > totalDistance && rayStart.y > fog1Top))
  {
    rayStartA = vec3(0);
    rayEndA = vec3(0);
  }
  // if (rayStartA.y > 0 && rayStartA.y < fog1Top)
  // {
  //   rayStartA = rayStart;
  // }

  if (rayStartA != vec3(0) && rayEndA != vec3(0))
  {
    //float deeznuts = distance(rayStartA, rayEndA);
    if (t_y2 > 0 && t_y2 < totalDistance) // cutoff is in view and in front of viewer
    {
      if (rayStart.y > fog1Top) // viewer above cutoff: clamp start of ray
      {
        rayStartA = rayStart + rayDir * (t_y2 + EPSILON);
        // fragColor = vec4(vec3(abs(rayStartA.y - u_heightOffset) / 100, 0, 0), 1);
        // return;
      }
      else // viewer below cutoff: clamp end of ray
      {
        rayEndA = rayStart + rayDir * t_y2;
        //fragColor = vec4(vec3(0, abs(rayStartA.y - u_heightOffset) / 100, 0), 1);
        //fragColor = vec4(vec3(0, abs(t_y2) / 100, 0), 1);
        //return;
      }
    }
    
    float magA = length(rayEndA - rayStartA);
    float y0 = rayStartA.y;
    float y1 = rayEndA.y;
    
    scatter += magA * (u_a - (y1 - y0) / (2.0 * u_b));
    //fragColor = vec4(clamp(magA / 10, 0, 1), 0, 0, 1);
    //fragColor = vec4(vec3(abs(rayStartA.y - u_heightOffset) / 100), 1);
    //fragColor = vec4(vec3(t_y2 / 100), 1);
    //return;
  }

  if (rayStartB != vec3(0) && rayEndB != vec3(0))
  {
    float magB = length(rayEndB - rayStartB);
    scatter += clamp(magB * u_fog2Density, 0.0, 1.0);
  }
  
  scatter = clamp(scatter, 0.0, 1.0);
  fragColor = vec4(mix(hdrColor, u_envColor, scatter), 1.0);

  //fragColor.xyz = fragColor.xyz * .0001 + rayEnd;
}