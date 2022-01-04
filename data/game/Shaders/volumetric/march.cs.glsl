#version 460 core
#include "../common.h"

layout(binding = 0) uniform sampler3D s_source;
layout(binding = 0) uniform writeonly image3D i_target;
layout(location = 0) uniform ivec3 u_targetDim;
layout(location = 1) uniform vec3 u_viewPos;
layout(location = 2) uniform mat4 u_invViewProj;
layout(location = 3) uniform float u_volNearPlane;
layout(location = 4) uniform float u_volFarPlane;

float beer(float d)
{
  return exp(-d);
}

float powder(float d)
{
  return 1.0 - exp(-d * 2.0);
}

layout(local_size_x = 16, local_size_y = 16) in;
void main()
{
  ivec2 gid = ivec2(gl_GlobalInvocationID.xy);
  if (any(greaterThanEqual(gid, u_targetDim.xy)))
    return;
  vec2 uv = (vec2(gid) + 0.5) / u_targetDim.xy;
  vec3 texel = 1.0 / u_targetDim;

  // RGB in-scattering + depth
  vec3 inScatteringAccum = vec3(0.0);
  float densityAccum = 0.0;
  vec3 pPrev = u_viewPos;
  for (int i = 0; i < u_targetDim.z; i++)
  {
    vec3 uvw = vec3(uv, (i + 0.5) / u_targetDim.z);
    float zInv = InvertDepthZO(uvw.z * uvw.z, u_volNearPlane, u_volFarPlane);
    vec3 pCur = UnprojectUV(zInv, uv, u_invViewProj);
    float d = distance(pPrev, pCur);
    pPrev = pCur;

    vec4 s = textureLod(s_source, uvw, 0);
    vec3 inScatteringPoint = s.rgb * s.a;
    densityAccum += s.a * d;
    float b = beer(densityAccum);
    float p = powder(densityAccum);
    inScatteringAccum += s.rgb * d * b * p * s.a;
    float transmittance = b;
    imageStore(i_target, ivec3(gid, i), vec4(inScatteringAccum, transmittance));
  }
}