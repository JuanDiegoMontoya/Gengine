#version 460 core
#define WORK_PER_THREAD 1
#define NUM_LOCAL_THREADS 256

layout (location = 1) uniform sampler2D u_hdrBuffer;

layout (std430, binding = 0) buffer in_data
{
  writeonly float idata[];
};

layout (local_size_x = NUM_LOCAL_THREADS) in;
void main()
{
  const uvec2 texSize = textureSize(u_hdrBuffer, 0);
  const uint numPixels = texSize.x * texSize.y;
  const uint numThreads = gl_NumWorkGroups.x * gl_WorkGroupSize.x;
  const uint start = gl_GlobalInvocationID.x * WORK_PER_THREAD;
  for (uint i = start; i < start + WORK_PER_THREAD && i < numPixels; i++)
  {
    ivec2 coords = ivec2(uint(i % texSize.x), uint(i / texSize.x));
    vec3 color = texelFetch(u_hdrBuffer, coords, 0).rgb;
    float log_lum = (dot(color, vec3(.3, .59, .11)));
    idata[i] = log_lum;
  }
}