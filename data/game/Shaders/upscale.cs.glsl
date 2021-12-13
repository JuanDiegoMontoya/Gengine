#version 460 core

layout(binding = 0) uniform sampler2D s_source;
layout(binding = 1) uniform sampler2D s_targetRead;
layout(binding = 0) uniform writeonly image2D i_targetWrite;
layout(location = 1) uniform float u_width = 1.0;
layout(location = 2) uniform float u_strength = 1.0 / 64.0;
layout(location = 3) uniform ivec2 u_sourceDim;
layout(location = 4) uniform ivec2 u_targetDim;

layout(local_size_x = 16, local_size_y = 16) in;
void main()
{
  ivec2 gid = ivec2(gl_GlobalInvocationID.xy);

  if (any(greaterThanEqual(gid, u_targetDim)))
    return;

  vec2 texel = 1.0 / u_sourceDim;

  // center of written pixel
  vec2 uv = (vec2(gid) + 0.5) / u_targetDim;

  vec4 rgba = texelFetch(s_targetRead, gid, 0);

  vec4 blurSum = vec4(0);
  blurSum += textureLod(s_source, uv + vec2(-1, -1) * texel * u_width, 0) * 1.0 / 16.0;
  blurSum += textureLod(s_source, uv + vec2(0, -1)  * texel * u_width, 0) * 2.0 / 16.0;
  blurSum += textureLod(s_source, uv + vec2(1, -1)  * texel * u_width, 0) * 1.0 / 16.0;
  blurSum += textureLod(s_source, uv + vec2(-1, 0)  * texel * u_width, 0) * 2.0 / 16.0;
  blurSum += textureLod(s_source, uv + vec2(0, 0)   * texel * u_width, 0) * 4.0 / 16.0;
  blurSum += textureLod(s_source, uv + vec2(1, 0)   * texel * u_width, 0) * 2.0 / 16.0;
  blurSum += textureLod(s_source, uv + vec2(-1, 1)  * texel * u_width, 0) * 1.0 / 16.0;
  blurSum += textureLod(s_source, uv + vec2(0, 1)   * texel * u_width, 0) * 2.0 / 16.0;
  blurSum += textureLod(s_source, uv + vec2(1, 1)   * texel * u_width, 0) * 1.0 / 16.0;
  rgba += blurSum * u_strength;

  imageStore(i_targetWrite, gid, rgba);
}