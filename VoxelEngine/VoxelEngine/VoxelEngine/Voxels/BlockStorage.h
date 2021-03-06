#pragma once
#include <Voxels/block.h>
#include <Utilities/BitArray.h>
#include <Utilities/Palette.h>
#include <array>

// abstract class for array-like block storage
class DenseBlockStorage
{
public:
  virtual Block GetBlock(int index) = 0;
  virtual BlockType GetBlockType(int index) = 0;
  virtual void SetBlockType(int index, BlockType) = 0;
  virtual void SetLight(int index, Light) = 0;
  virtual Light GetLight(int index) = 0;
};

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
  //Block* blocks_ = nullptr;
};


// https://www.reddit.com/r/VoxelGameDev/comments/9yu8qy/palettebased_compression_for_chunked_discrete/
// compressed block storage
// can't really return references w/o doing crazy proxy class stuff
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

  ConcurrentPalette<BlockType, Size> pblock_;
  ConcurrentPalette<Light, Size> plight_;
};

#include "BlockStorage.inl"