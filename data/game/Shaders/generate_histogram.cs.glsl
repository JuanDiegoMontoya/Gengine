#version 460 core
#define NUM_BUCKETS 128
#define LOCAL_X 16
#define LOCAL_Y 8
#define LOCAL_Z 1
#define WORKGROUPSIZE LOCAL_X * LOCAL_Y * LOCAL_Z

layout(location = 0) uniform sampler2D u_hdrBuffer;
layout(location = 1) uniform float u_logLowLum;
layout(location = 2) uniform float u_logMaxLum;

layout(std430, binding = 0) coherent restrict buffer Histogram
{
  int buckets[NUM_BUCKETS];
}histogram;

float map(float val, float r1s, float r1e, float r2s, float r2e)
{
  return (val - r1s) / (r1e - r1s) * (r2e - r2s) + r2s;
}

uint colorToBucket(vec3 color)
{
  float luminance = dot(color, vec3(.3, .59, .11));
  if (luminance < .0001)
    return 0;
  return clamp(int(map(log(luminance), u_logLowLum, u_logMaxLum, 0.0, float(NUM_BUCKETS - 1))), 0, NUM_BUCKETS - 1);
}

shared int shared_buckets[NUM_BUCKETS];

layout (local_size_x = LOCAL_X, local_size_y = LOCAL_Y, local_size_z = LOCAL_Z) in;
void main()
{
  shared_buckets[gl_LocalInvocationIndex] = 0;
  
  barrier();
  memoryBarrierShared();

  uvec2 texSize = textureSize(u_hdrBuffer, 0);
  vec2 upscaleFactor = vec2(texSize) / (gl_NumWorkGroups.xy * gl_WorkGroupSize.xy);
  uvec2 coords = uvec2(gl_GlobalInvocationID.xy * upscaleFactor);
  if (!any(greaterThanEqual(coords, texSize)))
  {
    vec3 color = texelFetch(u_hdrBuffer, ivec2(coords), 0).rgb;
    uint bucket = colorToBucket(color);
    atomicAdd(shared_buckets[bucket], 1);
  }

  barrier();
  memoryBarrierShared();

  atomicAdd(histogram.buckets[gl_LocalInvocationIndex], shared_buckets[gl_LocalInvocationIndex]);
}