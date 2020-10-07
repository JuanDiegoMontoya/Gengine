#include <Chunks/Chunk.h>

// applies multiple layers of compression onto a chunk
class CompressedChunk
{
public:
	CompressedChunk(const Chunk& chunk);
	~CompressedChunk() = default;

	// 1. reads chunk data
	// 2. store voxel + light data separately (and indices/positions for both)
	// 3. convert to bitmask + 8 voxel sub-chunk
	// 4. remove empty/air data and corresponding indices+bitmask flags
	// 5. delta compress subchunk indices
	// 6. RLE compress data
	

private:
	
	
};