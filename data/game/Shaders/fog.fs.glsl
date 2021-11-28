#version 460 core
#include "common.h"

#define EPSILON 1e-4

layout (location = 0) in vec2 vTexCoord;

layout(binding = 0) uniform sampler2D u_hdrColor;
layout(binding = 1) uniform sampler2D u_hdrDepth;
layout(location = 2) uniform mat4 u_invViewProj;
layout(location = 3) uniform ivec2 u_viewportSize;
layout(location = 4) uniform float u_distanceScale = 0.10;
layout(location = 5) uniform float u_heightOffset = -40.0;
layout(location = 6) uniform float u_hfIntensity = 1.0;
layout(location = 7) uniform vec3 u_envColor = vec3(1.0);
layout(location = 8) uniform float u_a = 0.1;
layout(location = 9) uniform float u_b = 50.0;
layout(location = 10) uniform float u_fog2Density = 50.0;
layout(location = 11) uniform float u_beer = 1.0;
layout(location = 12) uniform float u_powder = 1.0;

layout (location = 0) out vec4 fragColor;


void main()
{
  const vec3 hdrColor = texelFetch(u_hdrColor, ivec2(gl_FragCoord.xy), 0).xyz;
  const float depth = texelFetch(u_hdrDepth, ivec2(gl_FragCoord.xy), 0).r;
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

  float accum = 0.0;

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
    
    // f(x, y, z) = a + b / y
    accum += max(magA * (u_a - (y1 - y0) / (2.0 * u_b)), 0.0);
    //accum += magA * (u_a - (y1 - y0) / (2.0 * u_b));
    //fragColor = vec4(clamp(magA / 10, 0, 1), 0, 0, 1);
    //fragColor = vec4(vec3(abs(rayStartA.y - u_heightOffset) / 100), 1);
    //fragColor = vec4(vec3(t_y2 / 100), 1);
    //return;
  }

  if (rayStartB != vec3(0) && rayEndB != vec3(0))
  {
    float magB = length(rayEndB - rayStartB);
    float y0 = rayStartB.y;
    float y1 = rayEndB.y;
    float x0 = rayStartB.x;
    float x1 = rayEndB.x;
    float z0 = rayStartB.z;
    float z1 = rayEndB.z;
    float a2 = 0.03;
    float b2 = 0.03;
    float c2 = 1.0;
    float d2 = 1.1;
    // f(x, y, z) = 1
    accum += max(magB * u_fog2Density, 0.0);
    //accum += magB * u_fog2Density;
    //accum += u_fog2Density * magB * pow(u_b / (d2 * (y0 - y1)), exp(-d2 * (-y0/b2 + a2)) - exp(-d2 * (-y1/b2 + a2)));

    // f(x, y, z) = a + b * sin(c * x)
    // f(x, y, z) = a + b * cos(c * z)
    // accum += magB * (1.0 / (x0 - x1) * (a2 * x0 - a2 * x1 + b2 / c2 * (-cos(c2 * x0) + cos(c2 * x1))));
    // accum += magB * (1.0 / (z0 - z1) * (a2 * z0 - a2 * z1 + b2 / c2 * (sin(c2 * z0) - sin(c2 * z1))));
  }
  
  //accum = clamp(accum, 0.0, 1.0);
  float beer = exp(-u_beer * accum);
  float powder = 1.0 - exp(-u_powder * accum * 2.0);
  
  float scatter = (1.0 - beer) * powder;
  scatter = clamp(scatter, 0.0, 1.0);
  fragColor = vec4(mix(hdrColor, u_envColor, scatter), 1.0);
}