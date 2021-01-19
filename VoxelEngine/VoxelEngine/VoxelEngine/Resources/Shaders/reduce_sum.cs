#version 460 core
// the following constant can and will be replaced on the CPU
#define WORKGROUP_SIZE 1024

layout (location = 0) uniform uint u_n;

layout (std430, binding = 0) buffer in_data
{
  readonly float idata[];
};

layout (std430, binding = 1) buffer out_data
{
  writeonly float odata[];
};

shared float sdata[WORKGROUP_SIZE];

// TODO: use subgroup ops
void warpReduce(uint tid)
{
  if (WORKGROUP_SIZE >= 64){ sdata[tid] += sdata[tid + 32]; memoryBarrierShared(); }
  if (WORKGROUP_SIZE >= 32){ sdata[tid] += sdata[tid + 16]; memoryBarrierShared(); }
  if (WORKGROUP_SIZE >= 16){ sdata[tid] += sdata[tid + 8]; memoryBarrierShared(); }
  if (WORKGROUP_SIZE >= 8) { sdata[tid] += sdata[tid + 4]; memoryBarrierShared(); }
  if (WORKGROUP_SIZE >= 4) { sdata[tid] += sdata[tid + 2]; memoryBarrierShared(); }
  if (WORKGROUP_SIZE >= 2) { sdata[tid] += sdata[tid + 1]; }
}

layout (local_size_x = WORKGROUP_SIZE, local_size_y = 1, local_size_z = 1) in;
void main()
{
  const uvec3 blockIdx = gl_WorkGroupID;
  const uvec3 threadIdx = gl_LocalInvocationID;
  const uvec3 gridDim = gl_NumWorkGroups;

  const uint tid = threadIdx.x;
  uint i = blockIdx.x * (WORKGROUP_SIZE * 2) + tid;
  const uint gridSize = WORKGROUP_SIZE * 2 * gridDim.x;
  sdata[tid] = 0;
  while (i < u_n)
  {
    sdata[tid] += idata[i] + idata[i + WORKGROUP_SIZE];
    i += gridSize;
  }
  barrier();
  if (WORKGROUP_SIZE >= 1024) { if (tid < 512) { sdata[tid] += sdata[tid + 256]; } barrier(); }
  if (WORKGROUP_SIZE >= 512) { if (tid < 256) { sdata[tid] += sdata[tid + 256]; } barrier(); }
  if (WORKGROUP_SIZE >= 256) { if (tid < 128) { sdata[tid] += sdata[tid + 128]; } barrier(); }
  if (WORKGROUP_SIZE >= 128) { if (tid < 64) { sdata[tid] += sdata[tid + 64]; } barrier(); }
  if (tid < 32) warpReduce(tid);
  if (tid == 0)
  {
    odata[blockIdx.x] = sdata[0];
  }
}