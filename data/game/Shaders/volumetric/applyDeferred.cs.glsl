#version 460 core
#include "../common.h"

layout(binding = 0) uniform sampler2D s_color;
layout(binding = 1) uniform sampler2D s_depth;
layout(binding = 2) uniform sampler3D s_volume;
layout(binding = 3) uniform sampler2D s_blueNoise;
layout(binding = 0) uniform writeonly image2D i_target;
layout(location = 0) uniform ivec2 u_targetDim;
layout(location = 1) uniform mat4 u_viewProjVolume;
layout(location = 2) uniform mat4 u_invViewProjScene;

layout(local_size_x = 16, local_size_y = 16) in;
void main()
{
  ivec2 gid = ivec2(gl_GlobalInvocationID.xy);
  if (any(greaterThanEqual(gid, u_targetDim)))
  {
    return;
  }
  vec2 uv = (vec2(gid) + 0.5) / u_targetDim;

  float z = texelFetch(s_depth, gid, 0).x;
  if (z == 0.0) z = .00001;
  vec3 p = UnprojectUV(z, uv, u_invViewProjScene);

  vec4 volumeClip = u_viewProjVolume * vec4(p, 1.0);
  volumeClip.xyz = clamp(volumeClip.xyz, -volumeClip.www, volumeClip.www);
  vec3 volumeUV = volumeClip.xyz / volumeClip.w;
  volumeUV.z = LinearizeDepthZO(volumeUV.z, 1.0, 100.0);
  volumeUV.xy = (volumeUV.xy * 0.5) + 0.5;
  vec3 offset = texelFetch(s_blueNoise, gid % textureSize(s_blueNoise, 0).xy, 0).xyz * 2. - 1.;
  volumeUV += offset / vec3(textureSize(s_volume, 0).xyz);
  //volumeUV = clamp(volumeUV, 0, 1);
  if (volumeClip.z / volumeClip.w > 1.0)
  {
    volumeUV.z = 1.0;
  }

  vec3 baseColor = texelFetch(s_color, gid, 0).xyz;
  vec3 volumeColor = textureLod(s_volume, volumeUV, 0.0).rgb;

  // stupid yolo "blend" for testing
  vec3 finalColor = baseColor + volumeColor;
  imageStore(i_target, gid, vec4(finalColor, 1.0));
}