#version 460 core
#define WORKGROUP_SIZE 256

layout (location = 0) uniform uint u_n;

layout (std430, binding = 0) buffer in_data
{
  readonly float idata[];
};

layout (std430, binding = 1) buffer out_data
{
  writeonly float odata[];
};

#if 1
shared float sdata[WORKGROUP_SIZE];

void warpReduce(uint tid, uint blockSize)
{
  if (blockSize >= 64) sdata[tid] += sdata[tid + 32];
  if (blockSize >= 32) sdata[tid] += sdata[tid + 16];
  if (blockSize >= 16) sdata[tid] += sdata[tid + 8];
  if (blockSize >= 8) sdata[tid] += sdata[tid + 4];
  if (blockSize >= 4) sdata[tid] += sdata[tid + 2];
  if (blockSize >= 2) sdata[tid] += sdata[tid + 1];
}

layout (local_size_x = WORKGROUP_SIZE, local_size_y = 1, local_size_z = 1) in;
void main()
{
  const uvec3 blockIdx = gl_WorkGroupID;
  const uvec3 threadIdx = gl_LocalInvocationID;
  const uvec3 gridDim = gl_NumWorkGroups;
  const uint blockSize = WORKGROUP_SIZE;

  const uint tid = threadIdx.x;
  uint i = blockIdx.x * (blockSize * 2) + tid;
  const uint gridSize = blockSize * 2 * gridDim.x;
  sdata[tid] = 0;
  while (i < u_n)
  {
    sdata[tid] += idata[i] + idata[i + blockSize];
    i += gridSize;
  }
  barrier();
  if (blockSize >= 512) { if (tid < 256) { sdata[tid] += sdata[tid + 256]; } barrier(); }
  if (blockSize >= 256) { if (tid < 128) { sdata[tid] += sdata[tid + 128]; } barrier(); }
  if (blockSize >= 128) { if (tid < 64) { sdata[tid] += sdata[tid + 64]; } barrier(); }
  if (tid < 32) warpReduce(tid, blockSize);
  if (tid == 0)
  {
    odata[blockIdx.x] = sdata[0];
  }
}

#else

// #define blockIdx gl_WorkGroupID;
// #define threadIdx gl_LocalInvocationID;
// #define blockDim gl_WorkGroupSize;

shared float sdata[WORKGROUP_SIZE];
void main()
{
	uint globalIndex = gl_LocalInvocationID.x + gl_WorkGroupSize.x * gl_WorkGroupID.x;
	uint i = gl_LocalInvocationID.x;
	if (globalIndex < u_n)
		sdata[i] = idata[globalIndex];
	else
		sdata[i] = 0;

	barrier();

	for (uint j = gl_WorkGroupSize.x / 2; j > 0; j >>= 1)
	{
		if (i < j)
			sdata[i] += sdata[i + j];
		barrier();
	}
	if (i == 0)
		odata[gl_WorkGroupID.x] = sdata[0];
}

#endif