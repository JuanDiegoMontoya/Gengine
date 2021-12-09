#version 460 core

layout(binding = 0) uniform sampler2D u_readImage;
layout(binding = 0) uniform writeonly image2D u_writeImage;
layout(location = 0) uniform int u_readImageMip;

layout(local_size_x = 16, local_size_y = 16) in;
void main()
{
  ivec2 readImageSize = textureSize(u_readImage, u_readImageMip);
  ivec2 writeImageSize = readImageSize / 2;
  ivec2 coords = ivec2(gl_GlobalInvocationID.xy);

  if (any(greaterThanEqual(coords, writeImageSize)))
    return;

  vec2 stepWidth = 1.0 / readImageSize;

  // double stepWidth because the image we're writing to is one mip higher (half resolution)
  // stepWidth is added to move us to the center of the higher mip's pixel
  vec2 uv = 2.0 * stepWidth * coords + stepWidth;

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

  for (int i = 0; i < 13; i++)
  {
    samples[i] = textureLod(u_readImage, uv + sampleOffsets[i] * stepWidth, u_readImageMip).rgb;
  }

  vec3 boxes[5];
  boxes[0] = (samples[3] + samples[4] + samples[8 ] + samples[9 ]) * 0.25 * 0.5;
  boxes[1] = (samples[0] + samples[1] + samples[5 ] + samples[6 ]) * 0.25 * 0.125;
  boxes[2] = (samples[1] + samples[2] + samples[6 ] + samples[7 ]) * 0.25 * 0.125;
  boxes[3] = (samples[5] + samples[6] + samples[10] + samples[11]) * 0.25 * 0.125;
  boxes[4] = (samples[6] + samples[7] + samples[11] + samples[12]) * 0.25 * 0.125;
  
  vec3 filterSum = vec3(0);
  
  for (int i = 0; i < 5; i++)
  {
    if (u_readImageMip == 0)
    {
      float luminance = dot(boxes[i].rgb, vec3(0.299, 0.587, 0.114));
      boxes[i] *= 1.0 / (1.0 + luminance);
    }
    filterSum += boxes[i];
  }


  //vec4 rgba = textureLod(u_readImage, uv, u_readImageMip);

  imageStore(u_writeImage, coords, vec4(filterSum, 1.0));
}