#include "ChunkSerialize.h"
#include <zlib.h>
#include <cereal/cereal.hpp>

#pragma optimize("", off)
CompressedChunk::CompressedChunk(const Chunk& chunk)
{
  auto blocks = chunk.storage.pblock_;
  auto lights = chunk.storage.plight_;

  // records positions of blocks in the chunk
  // indices represent start of sequence of 8 blocks, at least one of which is NOT empty
  // bitmasks represent which blocks in an 8-block sequence is NOT empty
  std::vector<uint8_t> blocksBitmasks(Chunk::CHUNK_SIZE_CUBED / 8, 0);
  std::vector<uint8_t> lightsBitmasks(Chunk::CHUNK_SIZE_CUBED / 8, 0);
  std::vector<uint16_t> blocksIndices(Chunk::CHUNK_SIZE_CUBED / 8, UINT16_MAX);
  std::vector<uint16_t> lightsIndices(Chunk::CHUNK_SIZE_CUBED / 8, UINT16_MAX);
  for (int i = 0; i < Chunk::CHUNK_SIZE_CUBED; i++)
  {
    if (blocks.GetVal(i) != BlockType::bAir)
    {
      blocksIndices[i / 8] = i - (i % 8);
      blocksBitmasks[i / 8] |= 1 << (i % 8);
    }
    if (lights.GetVal(i) != Light{})
    {
      lightsIndices[i / 8] = i - (i % 8);
      lightsBitmasks[i / 8] |= 1 << (i % 8);
    }
  }
  std::erase(blocksBitmasks, 0);
  std::erase(lightsBitmasks, 0);
  std::erase(blocksIndices, UINT16_MAX);
  std::erase(lightsIndices, UINT16_MAX);

  // get indices of empty entries in palettes
  int blockIndexLen = blocks.paletteEntryLength_;
  int lightIndexLen = lights.paletteEntryLength_;
  int emptyBlockIndex = std::find(blocks.palette_.begin(), blocks.palette_.end(), BlockType::bAir) - blocks.palette_.begin();
  int emptyLightIndex = std::find(lights.palette_.begin(), lights.palette_.end(), Light{}) - lights.palette_.begin();

  // remove empty entries from palettes and from data
  const int blockToRem = blocks.palette_[emptyBlockIndex].refcount;
  const int lightToRem = lights.palette_[emptyLightIndex].refcount;
  blocks.palette_.erase(blocks.palette_.begin() + emptyBlockIndex);
  lights.palette_.erase(lights.palette_.begin() + emptyBlockIndex);
  auto remdb = blocks.data_.EraseAll(blockIndexLen, emptyBlockIndex);
  auto remdl = lights.data_.EraseAll(lightIndexLen, emptyLightIndex);
  ASSERT(remdb == blockToRem);
  ASSERT(remdl == lightToRem);


}
