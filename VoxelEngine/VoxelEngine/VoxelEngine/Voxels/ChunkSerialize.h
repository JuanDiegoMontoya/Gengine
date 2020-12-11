#pragma once
#include <Voxels/Chunk.h>

struct CompressedChunkData
{
  std::vector<std::byte> data;
};

// applies multiple layers of compression onto a chunk
// 1. reads chunk data
// 2. store voxel + light data separately (and indices/positions for both)
// 3. convert to bitmask + 8 voxel sub-chunk
// 4. remove empty/air data and corresponding indices+bitmask flags
// 5. delta compress subchunk indices
// 6. RLE compress data
CompressedChunkData CompressChunk(PaletteBlockStorage<Chunk::CHUNK_SIZE_CUBED> data);
PaletteBlockStorage<Chunk::CHUNK_SIZE_CUBED> DecompressChunk(CompressedChunkData);
