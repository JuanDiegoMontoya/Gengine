#include "vPCH.h"
#include "ChunkSerialize.h"
#include <zlib/zlib.h>
#include <cereal/cereal.hpp>
#include <utility/Compression.h>
#include <utility/serialize.h>

#include <cereal/types/vector.hpp>
#include <cereal/archives/binary.hpp>
#include <sstream>
#include <voxel/Chunk.h>

namespace Voxels
{
  template<typename T>
  struct CompressedMaterialInfo
  {
    CompressedMaterialInfo(Palette<T, Voxels::Chunk::CHUNK_SIZE_CUBED> p) :
      indices(Chunk::CHUNK_SIZE_CUBED / 8, UINT16_MAX),
      bitmasks(Chunk::CHUNK_SIZE_CUBED / 8, 0),
      palette(p)
    {
    }

    std::vector<int16_t> indices;
    std::vector<uint8_t> bitmasks;
    Palette<T, Voxels::Chunk::CHUNK_SIZE_CUBED> palette;

    void MakeIndicesAndBitmasks(const T& emptyVal)
    {
      // records positions of blocks in the chunk
      // indices represent start of sequence of 8 blocks, at least one of which is NOT empty
      // bitmasks represent which blocks in an 8-block sequence is NOT empty
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
      size_t indexLen = palette.paletteEntryLength_;
      size_t emptyIndex = std::find(palette.palette_.begin(), palette.palette_.end(), emptyVal) - palette.palette_.begin();

      // remove empty entries from palettes and from data
      const size_t toRemove = palette.palette_[emptyIndex].refcount;
      palette.palette_.erase(palette.palette_.begin() + emptyIndex);
      auto prevSize = palette.data_.size();
      palette.data_ = palette.data_.FindAll(indexLen, [emptyIndex](auto n) { return n != emptyIndex; });
      auto remdb = (prevSize - palette.data_.size()) / indexLen;
      ASSERT(remdb == toRemove);
    }
  };

  CompressedChunkData CompressChunk(PaletteBlockStorage<Voxels::Chunk::CHUNK_SIZE_CUBED> data)
  {
    auto blocks = data.pblock_;
    auto lights = data.plight_;

    CompressedMaterialInfo<BlockType> blockData(blocks);
    CompressedMaterialInfo<Light> lightData(lights);
    blockData.MakeIndicesAndBitmasks(BlockType::bAir);
    blockData.RemoveEmptyPaletteData(BlockType::bAir);
    lightData.MakeIndicesAndBitmasks(Light{});
    lightData.RemoveEmptyPaletteData(Light{});
    auto bytesA = blockData.palette.GetData().ByteRepresentation();

    auto deltaA = Compression::EncodeDelta<int16_t>(blockData.indices);
    auto deltaB = Compression::EncodeDelta<int16_t>(lightData.indices);

    auto rleA = Compression::EncodeRLE<int16_t>(deltaA);
    auto rleB = Compression::EncodeRLE<int16_t>(deltaB);
    
    auto compressedA = Compression::Compress<Compression::RLEelement<int16_t>>(rleA);
    auto compressedB = Compression::Compress<Compression::RLEelement<int16_t>>(rleB);

#if 1
    // tests
    auto bitsA = BitArray(bytesA);
    ASSERT(bitsA == blockData.palette.GetData());
    auto ddataA = Compression::DecodeDelta<int16_t>(deltaA);
    ASSERT(ddataA == blockData.indices);
    auto rdataA = Compression::DecodeRLE<int16_t>(rleA);
    ASSERT(deltaA == rdataA);
    auto uncompressA = Compression::Uncompress(compressedA);
    ASSERT(uncompressA == rleA);
#endif

    std::stringstream binaryData;
    cereal::BinaryOutputArchive archive(binaryData);
    archive(compressedA, compressedB);

    CompressedChunkData ret;
    std::string outStr = binaryData.str();
    ret.data.reserve(outStr.size());
    std::for_each(outStr.begin(), outStr.end(), [&ret](char c) { ret.data.push_back((std::byte)c); });
    return ret;
  }

  PaletteBlockStorage<Voxels::Chunk::CHUNK_SIZE_CUBED> DecompressChunk(CompressedChunkData data)
  {
    std::stringstream inStr;
    for (auto b : data.data)
    {
      inStr << (char)b;
    }
    cereal::BinaryInputArchive archive(inStr);
    Compression::CompressionResult<Compression::RLEelement<uint16_t>> resA;
    Compression::CompressionResult<Compression::RLEelement<uint16_t>> resB;
    archive(resA, resB);

    ASSERT(false);
    PaletteBlockStorage<Voxels::Chunk::CHUNK_SIZE_CUBED> bb;
    return bb;
  }
}