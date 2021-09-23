#version 460 core

#include "common.h"

layout(location = 0) in vec2 vTexCoord;

layout(location = 0) out vec4 fragColor;

layout(binding = 0) uniform sampler2D u_screenDepth;
layout(binding = 1) uniform sampler2D u_screenRoughnessMetalness;
layout(binding = 2) uniform samplerCube u_ReflectionCubemap;
layout(binding = 3) uniform samplerCube u_ReflectionCubemapDistance;
layout(binding = 4) uniform samplerCube u_SkyCube;
layout(binding = 5) uniform sampler2D u_blueNoise;

layout(location = 0) uniform mat4 u_invProj;
layout(location = 1) uniform mat4 u_invView;
layout(location = 2) uniform vec3 u_viewPos;

vec3 GetNormal(vec3 pos)
{
  return normalize(cross(dFdxFine(pos), dFdyFine(pos)));
}

void main()
{
  // TODO: sample env cube directly... or something
}