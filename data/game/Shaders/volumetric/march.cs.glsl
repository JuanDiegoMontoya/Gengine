#version 460 core
#include "../common.h"

layout(binding = 0) uniform sampler3D s_source;
layout(binding = 0) uniform writeonly image3D i_target;
layout(location = 0) uniform ivec3 u_targetDim;
layout(location = 1) uniform vec3 u_viewPos;
layout(location = 2) uniform mat4 u_invViewProj;

layout(local_size_x = 16, local_size_y = 16) in;
void main()
{
  ivec2 gid = ivec2(gl_GlobalInvocationID.xy);
  if (any(greaterThanEqual(gid, u_targetDim.xy)))
    return;
  vec2 uv = (vec2(gid) + 0.5) / u_targetDim.xy;
  vec3 texel = 1.0 / u_targetDim;

  vec3 accum = vec3(0.0);
  vec3 pPrev = u_viewPos;
  for (int i = 0; i < u_targetDim.z; i++)
  {
    vec3 uvw = vec3(uv, (i + 0.5) / u_targetDim.z);
    float zInv = InvertDepthZO(uvw.z, 1.0, 100.0);
    vec3 pCur = UnprojectUV(zInv, uv, u_invViewProj);
    float d = distance(pPrev, pCur);
    pPrev = pCur;
    accum += textureLod(s_source, uvw, 0).rgb * d;
    imageStore(i_target, ivec3(gid, i), vec4(accum, 1.0));
    //float c = float(i) / u_targetDim.z;
    //vec3 t = mix(vec3(0, 1, 0), vec3(1, 0, 0), c);
    //vec3 t = vec3(0, 1, 0);
    //if (c > .5) t = vec3(1, 0, 0);
    //imageStore(i_target, ivec3(gid, i), vec4(accum * .0001 + t, 1));
  }
}