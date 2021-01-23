#version 460 core
#define NUM_BUCKETS 128

layout (location = 0) uniform sampler2D u_hdrBuffer;
layout (location = 1) uniform float u_logLowLum;
layout (location = 2) uniform float u_logMaxLum;

layout (std430, binding = 0) buffer histogram
{
  coherent int buckets[NUM_BUCKETS];
};

float map(float val, float r1s, float r1e, float r2s, float r2e)
{
  return (val - r1s) / (r1e - r1s) * (r2e - r2s) + r2s;
}

layout (local_size_x = 32, local_size_y = 32, local_size_z = 1) in;
void main()
{
  uvec2 texSize = textureSize(u_hdrBuffer, 0);
  uvec2 coords = gl_GlobalInvocationID.xy;
  if (any(greaterThanEqual(coords, texSize))) return;
  vec3 color = texelFetch(u_hdrBuffer, ivec2(coords), 0).rgb;
  float luminance = dot(color, vec3(.3, .59, .11));
  int bucket = clamp(int(map(log(luminance), u_logLowLum, u_logMaxLum, 0.0, float(NUM_BUCKETS - 1))), 0, NUM_BUCKETS - 1);
  atomicAdd(buckets[bucket], 1);
}