#version 460 core

layout(binding = 0) uniform sampler2D u_readImageBlur; // another image
layout(binding = 1) uniform sampler2D u_readImageMip; // same image as what's being written to
layout(binding = 0) uniform writeonly image2D u_writeImage;
layout(location = 0) uniform int u_writeImageMip;
layout(location = 1) uniform bool u_firstPass;
layout(location = 2) uniform bool u_lastPass; // no relation
layout(location = 3) uniform float u_blurWidth;

layout(local_size_x = 16, local_size_y = 16) in;
void main()
{
  int readImageMip = u_writeImageMip + 1;
  ivec2 writeImageSize = imageSize(u_writeImage);
  ivec2 coords = ivec2(gl_GlobalInvocationID.xy);

  if (any(greaterThanEqual(coords, writeImageSize)))
    return;

  vec2 stepWidth = 1.0 / writeImageSize;

  // center of written pixel
  vec2 uv = stepWidth * coords + stepWidth / 2.0;

  vec4 rgba = vec4(0);
  
  if (!u_firstPass)
  {
    rgba += textureLod(u_readImageMip, uv, readImageMip);
  }

  if (!u_lastPass)
  {
    const mat3 blurWeights = 
    {
      { 1, 2, 1 },
      { 2, 4, 2 },
      { 1, 2, 1 }
    };

    vec4 blurSum = vec4(0);
    for (int y = -1; y <= 1; y++)
    {
      for (int x = -1; x <= 1; x++)
      {
        blurSum += textureLod(u_readImageBlur, uv + vec2(x, y) * stepWidth * u_blurWidth, u_writeImageMip) * blurWeights[y + 1][x + 1];
      }
    }
    rgba += blurSum / 16.0;
  }
  else
  {
    vec4 destColor = texelFetch(u_readImageMip, coords, u_writeImageMip);
    rgba += destColor;
  }

  if (!u_firstPass)
    rgba *= 0.5;

  imageStore(u_writeImage, coords, rgba);
}