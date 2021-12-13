#version 460 core

layout(binding = 0) uniform sampler2D s_source;
layout(binding = 0) uniform writeonly image2D i_target;
layout(location = 0) uniform ivec2 u_sourceDim;
layout(location = 1) uniform ivec2 u_targetDim;

float luma(vec3 c)
{
  return dot(c, vec3(0.299, 0.587, 0.114));
}

vec3 karisAverage(vec3 c1, vec3 c2, vec3 c3, vec3 c4)
{
	float c1w = 1.0 / (luma(c1.rgb) + 1.0);
	float c2w = 1.0 / (luma(c2.rgb) + 1.0);
	float c3w = 1.0 / (luma(c3.rgb) + 1.0);
	float c4w = 1.0 / (luma(c4.rgb) + 1.0);
	
	return (c1 * c1w + c2 * c2w + c3 * c3w + c4 * c4w) / (c1w + c2w + c3w + c4w);	
}

layout(local_size_x = 16, local_size_y = 16) in;
void main()
{
  ivec2 gid = ivec2(gl_GlobalInvocationID.xy);

  if (any(greaterThanEqual(gid, u_targetDim)))
    return;

  vec2 texel = 1.0 / u_sourceDim;

  // center of written pixel
  vec2 uv = (vec2(gid) + 0.5) / u_targetDim;

  // for reference
  vec2 sampleOffsets[13] =
  {
    { -2, -2 }, // 0
    { 0, -2 },
    { 2, -2 },  // 2
    { -1, -1 },
    { 1, -1 },  // 4
    { -2, 0 },
    { 0, 0 },   // 6
    { 2, 0 },
    { -1, 1 },  // 8
    { 1, 1 },
    { -2, 2 },  // 10
    { 0, 2 },
    { 2, 2 }    // 12
  };

  vec3 samples[13];
  samples[0 ] = textureLod(s_source, uv + texel * vec2(-2, -2), 0).rgb;
  samples[1 ] = textureLod(s_source, uv + texel * vec2(0, -2) , 0).rgb;
  samples[2 ] = textureLod(s_source, uv + texel * vec2(2, -2) , 0).rgb;
  samples[3 ] = textureLod(s_source, uv + texel * vec2(-1, -1), 0).rgb;
  samples[4 ] = textureLod(s_source, uv + texel * vec2(1, -1) , 0).rgb;
  samples[5 ] = textureLod(s_source, uv + texel * vec2(-2, 0) , 0).rgb;
  samples[6 ] = textureLod(s_source, uv + texel * vec2(0, 0)  , 0).rgb;
  samples[7 ] = textureLod(s_source, uv + texel * vec2(2, 0)  , 0).rgb;
  samples[8 ] = textureLod(s_source, uv + texel * vec2(-1, 1) , 0).rgb;
  samples[9 ] = textureLod(s_source, uv + texel * vec2(1, 1)  , 0).rgb;
  samples[10] = textureLod(s_source, uv + texel * vec2(-2, 2) , 0).rgb;
  samples[11] = textureLod(s_source, uv + texel * vec2(0, 2)  , 0).rgb;
  samples[12] = textureLod(s_source, uv + texel * vec2(2, 2)  , 0).rgb;

  vec3 filterSum = vec3(0);
  filterSum += karisAverage(samples[3], samples[4], samples[8 ], samples[9 ]) * 0.5;
  filterSum += karisAverage(samples[0], samples[1], samples[5 ], samples[6 ]) * 0.125;
  filterSum += karisAverage(samples[1], samples[2], samples[6 ], samples[7 ]) * 0.125;
  filterSum += karisAverage(samples[5], samples[6], samples[10], samples[11]) * 0.125;
  filterSum += karisAverage(samples[6], samples[7], samples[11], samples[12]) * 0.125;
  
  //filterSum = textureLod(u_readImage, uv, u_readImageMip).rgb;

  imageStore(i_target, gid, vec4(filterSum, 1.0));
}