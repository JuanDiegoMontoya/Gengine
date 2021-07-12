#version 460 core
#define NUM_BUCKETS 128

layout (std430, binding = 0) buffer exposures
{
  float readExposure;
  float writeExposure;
};

layout (std430, binding = 1) buffer histogram
{
  coherent int buckets[NUM_BUCKETS];
};

layout (location = 0) uniform float u_dt;
layout (location = 1) uniform float u_adjustmentSpeed;
layout (location = 2) uniform float u_logLowLum;
layout (location = 3) uniform float u_logMaxLum;
layout (location = 4) uniform float u_targetLuminance = 0.22;
layout (location = 5) uniform int u_numPixels;

float map(float val, float r1s, float r1e, float r2s, float r2e)
{
  return (val - r1s) / (r1e - r1s) * (r2e - r2s) + r2s;
}

// TODO: test performance of this kernel vs simply running it on the CPU
layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
void main()
{
  float temp = readExposure;
  readExposure = writeExposure;
  writeExposure = temp;

  uint sum = 0;
  for (int i = 0; i < NUM_BUCKETS; i++)
  {
    sum += buckets[i] * (i + 1);
    buckets[i] = 0;
  }
  
  float meanLuminance = exp(map(float(sum) / float(u_numPixels), 0.0, NUM_BUCKETS, u_logLowLum, u_logMaxLum));
  float exposureTarget = u_targetLuminance / meanLuminance;
  writeExposure = mix(readExposure, exposureTarget, u_dt * u_adjustmentSpeed);
}