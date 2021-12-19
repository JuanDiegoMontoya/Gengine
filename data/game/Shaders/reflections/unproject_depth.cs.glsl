#version 460 core
#include "../common.h"

layout(local_size_x = 16, local_size_y = 16) in;

layout(location = 0) uniform mat4 u_invProj;
layout(location = 1) uniform ivec2 u_targetDim;
layout(binding = 0) uniform sampler2D s_depth;
layout(binding = 0) uniform restrict writeonly image2D i_target;

void main()
{
  ivec2 gid = ivec2(gl_GlobalInvocationID.xy);
  if (any(greaterThanEqual(gid, u_targetDim)))
    return;
  vec2 uv = (vec2(gid) + 0.5) / u_targetDim;
  float depthSample = texelFetch(s_depth, gid, 0).x;
  imageStore(i_target, gid, vec4(length(UnprojectUV(depthSample, uv, u_invProj))));
}