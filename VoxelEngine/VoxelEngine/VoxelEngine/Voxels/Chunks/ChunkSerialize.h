#include <Chunks/Chunk.h>

// applies multiple layers of compression onto a chunk
class CompressedChunk
{
public:
  CompressedChunk(const Chunk& chunk)
  {
    auto blocks = chunk.storage.pblock_;
    auto lights = chunk.storage.plight_;

    std::vector<uint16_t> blocksIndices;
    std::vector<uint16_t> lightsIndices;

    // remove all air+empty entries from the palettes, then resize
    for (int i = 0; i < Chunk::CHUNK_SIZE_CUBED; i++)
    {
    }
    //blocks.fitPalette();
  }
  ~CompressedChunk() = default;

  // 1. reads chunk data
  // 2. store voxel + light data separately (and indices/positions for both)
  // 3. convert to bitmask + 8 voxel sub-chunk
  // 4. remove empty/air data and corresponding indices+bitmask flags
  // 5. delta compress subchunk indices
  // 6. RLE compress data
  
  void ReadChunkData();

private:
  
};