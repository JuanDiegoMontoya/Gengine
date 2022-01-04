#version 460 core
#include "common.h"

#define EPSILON 1e-4

layout(binding = 0) uniform sampler2D s_color;
layout(binding = 1) uniform sampler2D s_depth;
layout(location = 2) uniform mat4 u_invViewProj;
layout(location = 4) uniform float u_distanceScale = 0.10;
layout(location = 5) uniform float u_heightOffset = -40.0;
layout(location = 6) uniform float u_hfIntensity = 1.0;
layout(location = 7) uniform vec3 u_envColor = vec3(1.0);
layout(location = 8) uniform float u_a = 0.1;
layout(location = 9) uniform float u_b = 50.0;
layout(location = 10) uniform float u_fog2Density = 50.0;
layout(location = 11) uniform float u_beer = 1.0;
layout(location = 12) uniform float u_powder = 1.0;
layout(location = 13) uniform ivec2 u_targetDim;

layout(binding = 0) uniform writeonly image2D i_target;

layout(local_size_x = 16, local_size_y = 16) in;
void main()
{
  ivec2 gid = ivec2(gl_GlobalInvocationID.xy);
  if (any(greaterThanEqual(gid, u_targetDim)))
    return;
  vec2 uv = (vec2(gid) + 0.5) / u_targetDim;

  const vec3 hdrColor = texelFetch(s_color, gid, 0).rgb;
  const float depth = texelFetch(s_depth, gid, 0).r;
  vec3 rayEnd = UnprojectUV(max(depth, .00001), uv, u_invViewProj);
  vec3 rayStart = UnprojectUV(1.0, uv, u_invViewProj);
  vec3 rayDir = normalize(rayEnd - rayStart);
  const float totalDistance = distance(rayStart, rayEnd);

  rayStart += u_heightOffset;
  rayEnd += u_heightOffset;

  if (abs(rayDir.y) < .00001)
    rayDir.y += .0001;

  // xz-plane intercept
  float t_y = -1e7;
  if (abs(rayDir.y) > .00001)
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

  if (rayStartA != vec3(0) && rayEndA != vec3(0))
  {
    if (t_y2 > 0 && t_y2 < totalDistance) // cutoff is in view and in front of viewer
    {
      if (rayStart.y > fog1Top) // viewer above cutoff: clamp start of ray
      {
        rayStartA = rayStart + rayDir * (t_y2 + EPSILON);
      }
      else // viewer below cutoff: clamp end of ray
      {
        rayEndA = rayStart + rayDir * t_y2;
      }
    }
    
    float magA = length(rayEndA - rayStartA);
    float y0 = rayStartA.y;
    float y1 = rayEndA.y;
    
    // f(x, y, z) = a + b / y
    accum += max(magA * (u_a - (y1 - y0) / (2.0 * u_b)), 0.0);
  }

  if (rayStartB != vec3(0) && rayEndB != vec3(0))
  {
    float magB = length(rayEndB - rayStartB);
    // f(x, y, z) = 1
    accum += max(magB * u_fog2Density, 0.0);
  }
  
  //accum = clamp(accum, 0.0, 1.0);
  float beer = exp(-u_beer * accum);
  float powder = 1.0 - exp(-u_powder * accum * 2.0);
  
  vec3 inScattering = u_envColor * beer * powder;
  vec3 finalColor = hdrColor * beer + inScattering;
  imageStore(i_target, gid, vec4(finalColor, 1.0));
}