#pragma once
#include <Voxels/block.h>
#include <Utilities/BitArray.h>
#include <Utilities/Palette.h>
#include <array>

// uncompressed block storage for chunks
template<unsigned Size>
class ArrayBlockStorage
{
public:
  // num blocks
  ArrayBlockStorage();
  ~ArrayBlockStorage();
  ArrayBlockStorage(const ArrayBlockStorage&);
  ArrayBlockStorage& operator=(const ArrayBlockStorage&);

  Block& operator[](int index);
  Block& GetBlockRef(int index);
  Block GetBlock(int index) const;
  BlockType GetBlockType(int index) const;
  void SetBlock(int index, BlockType);
  void SetLight(int index, Light);
  Light GetLight(int index) const;

private:

  std::array<Block, Size> blocks_;
};

// palette-encoded block storage for efficient memory usage
template<unsigned Size>
class PaletteBlockStorage
{
public:
  void SetBlock(int index, BlockType);
  Block GetBlock(int index) const;
  BlockType GetBlockType(int index) const;
  void SetLight(int index, Light);
  Light GetLight(int index) const;

  PaletteBlockStorage& operator=(const PaletteBlockStorage& other)
  {
    pblock_ = other.pblock_;
    plight_ = other.plight_;
    return *this;
  }

  template <class Archive>
  void serialize(Archive& ar)
  {
    ar(pblock_);
  }

  Palette<BlockType, Size> pblock_;
  Palette<Light, Size> plight_;
};

#include "BlockStorage.inl"