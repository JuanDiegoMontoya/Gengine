#version 460 core

layout(binding = 0) uniform sampler2D s_source;
layout(binding = 0) uniform writeonly image2D i_target;
layout(location = 0) uniform ivec2 u_sourceDim;
layout(location = 1) uniform ivec2 u_targetDim;
layout(location = 2) uniform float u_sourceLod;

layout(local_size_x = 16, local_size_y = 16) in;
void main()
{
  ivec2 gid = ivec2(gl_GlobalInvocationID.xy);

  if (any(greaterThanEqual(gid, u_targetDim)))
    return;

  vec2 texel = 1.0 / u_sourceDim;

  // center of written pixel
  vec2 uv = (vec2(gid) + 0.5) / u_targetDim;

  vec3 filterSum = vec3(0);
  filterSum += textureLod(s_source, uv + texel * vec2(-2, -2), u_sourceLod).rgb * (1.0 / 32.0);
  filterSum += textureLod(s_source, uv + texel * vec2(2, -2) , u_sourceLod).rgb * (1.0 / 32.0);
  filterSum += textureLod(s_source, uv + texel * vec2(-2, 2) , u_sourceLod).rgb * (1.0 / 32.0);
  filterSum += textureLod(s_source, uv + texel * vec2(2, 2)  , u_sourceLod).rgb * (1.0 / 32.0);
  filterSum += textureLod(s_source, uv + texel * vec2(0, 2)  , u_sourceLod).rgb * (2.0 / 32.0);
  filterSum += textureLod(s_source, uv + texel * vec2(0, -2) , u_sourceLod).rgb * (2.0 / 32.0);
  filterSum += textureLod(s_source, uv + texel * vec2(2, 0)  , u_sourceLod).rgb * (2.0 / 32.0);
  filterSum += textureLod(s_source, uv + texel * vec2(-2, 0) , u_sourceLod).rgb * (2.0 / 32.0);
  filterSum += textureLod(s_source, uv + texel * vec2(0, 0)  , u_sourceLod).rgb * (4.0 / 32.0);
  filterSum += textureLod(s_source, uv + texel * vec2(-1, -1), u_sourceLod).rgb * (4.0 / 32.0);
  filterSum += textureLod(s_source, uv + texel * vec2(1, -1) , u_sourceLod).rgb * (4.0 / 32.0);
  filterSum += textureLod(s_source, uv + texel * vec2(-1, 1) , u_sourceLod).rgb * (4.0 / 32.0);
  filterSum += textureLod(s_source, uv + texel * vec2(1, 1)  , u_sourceLod).rgb * (4.0 / 32.0);

  //filterSum = textureLod(u_readImage, uv, u_readImageMip).rgb;

  imageStore(i_target, gid, vec4(filterSum, 1.0));
}