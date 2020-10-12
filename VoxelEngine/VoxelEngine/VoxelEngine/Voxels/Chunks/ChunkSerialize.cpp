#include "ChunkSerialize.h"
#include <zlib.h>
#include <cereal/cereal.hpp>
#include <Utilities/DeltaEncoder.h>
#include <Utilities/RunLengthEncoder.h>
#include <Utilities/CompressBuffer.h>

template<typename T>
struct CompressedMaterialInfo
{
  CompressedMaterialInfo(Palette<T, Chunk::CHUNK_SIZE_CUBED> p) :
    indices(Chunk::CHUNK_SIZE_CUBED / 8, UINT16_MAX),
    bitmasks(Chunk::CHUNK_SIZE_CUBED / 8, 0),
    palette(p) {}

  std::vector<uint16_t> indices;
  std::vector<uint8_t> bitmasks;
  Palette<T, Chunk::CHUNK_SIZE_CUBED> palette;

  void MakeIndicesAndBitmasks(const T& emptyVal)
  {
    for (int i = 0; i < Chunk::CHUNK_SIZE_CUBED; i++)
    {
      if (palette.GetVal(i) != emptyVal)
      {
        indices[i / 8] = i - (i % 8);
        bitmasks[i / 8] |= 1 << (i % 8);
      }
    }
    std::erase(indices, UINT16_MAX);
    std::erase(bitmasks, 0);
  }

  void RemoveEmptyPaletteData(const T& emptyVal)
  {
    // get indices of empty entries in palettes
    int indexLen = palette.paletteEntryLength_;
    int emptyIndex = std::find(palette.palette_.begin(), palette.palette_.end(), emptyVal) - palette.palette_.begin();

    // remove empty entries from palettes and from data
    const int toRemove = palette.palette_[emptyIndex].refcount;
    palette.palette_.erase(palette.palette_.begin() + emptyIndex);
    auto prevSize = palette.data_.size();
    palette.data_ = palette.data_.FindAll(indexLen, [emptyIndex](auto n) { return n != emptyIndex; });
    auto remdb = (prevSize - palette.data_.size()) / indexLen;
    ASSERT(remdb == toRemove);
  }
};

CompressedChunk::CompressedChunk(const Chunk& chunk)
{
  auto blocks = chunk.storage.pblock_;
  auto lights = chunk.storage.plight_;

  // records positions of blocks in the chunk
  // indices represent start of sequence of 8 blocks, at least one of which is NOT empty
  // bitmasks represent which blocks in an 8-block sequence is NOT empty
  CompressedMaterialInfo<BlockType> blockData(blocks);
  CompressedMaterialInfo<Light> lightData(lights);
  blockData.MakeIndicesAndBitmasks(BlockType::bAir);
  lightData.MakeIndicesAndBitmasks(Light{});

  blockData.RemoveEmptyPaletteData(BlockType::bAir);
  lightData.RemoveEmptyPaletteData(Light{});

  auto deltaA = Compression::EncodeDelta(std::span(blockData.indices.data(), blockData.indices.size()));
  auto ddataA = Compression::DecodeDelta(std::span(deltaA.data(), deltaA.size()));
  ASSERT(ddataA == blockData.indices);
  
  auto rleA = Compression::EncodeRLE(std::span(deltaA.data(), deltaA.size()));
  auto rdataA = Compression::DecodeRLE(std::span(rleA.data(), rleA.size()));
  ASSERT(deltaA == rdataA);

  auto compressedA = Compression::Compress(std::span(rleA.data(), rleA.size()));
}
