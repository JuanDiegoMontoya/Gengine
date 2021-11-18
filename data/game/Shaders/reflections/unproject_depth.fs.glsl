#version 460 core

#include "../common.h"

layout(binding = 0) uniform sampler2D u_depthTex;
layout(location = 1) uniform mat4 u_invViewProj;
layout(location = 2) uniform vec3 u_viewPos;

layout(location = 0) in vec2 vTexCoord;

layout(location = 0) out float depth;

void main()
{
  float depthSample = texelFetch(u_depthTex, ivec2(gl_FragCoord.xy), 0).x;
  depth = distance(u_viewPos, WorldPosFromDepthUV(depthSample, vTexCoord, u_invViewProj));
}