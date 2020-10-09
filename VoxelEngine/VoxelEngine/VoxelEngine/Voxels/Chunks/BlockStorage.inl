#pragma once
#include <Chunks/BlockStorage.h>

template<unsigned Size>
inline ArrayBlockStorage<Size>::ArrayBlockStorage()
{
}

template<unsigned Size>
inline ArrayBlockStorage<Size>::~ArrayBlockStorage()
{
}

template<unsigned Size>
inline ArrayBlockStorage<Size>::ArrayBlockStorage(const ArrayBlockStorage& other)
{
  *this = other;
}

template<unsigned Size>
inline ArrayBlockStorage<Size>& ArrayBlockStorage<Size>::operator=(const ArrayBlockStorage& other)
{
  this->blocks = other.blocks;
  return *this;
}

template<unsigned Size>
inline Block& ArrayBlockStorage<Size>::operator[](int index)
{
  return blocks_[index];
}

template<unsigned Size>
inline Block& ArrayBlockStorage<Size>::GetBlockRef(int index)
{
  return blocks_[index];
}

template<unsigned Size>
inline Block ArrayBlockStorage<Size>::GetBlock(int index) const
{
  return blocks_[index];
}

template<unsigned Size>
inline BlockType ArrayBlockStorage<Size>::GetBlockType(int index) const
{
  return blocks_[index].GetType();
}

template<unsigned Size>
inline void ArrayBlockStorage<Size>::SetBlock(int index, BlockType type)
{
  blocks_[index].SetType(type);
}

template<unsigned Size>
inline void ArrayBlockStorage<Size>::SetLight(int index, Light light)
{
  blocks_[index].GetLightRef() = light;
}

template<unsigned Size>
inline Light ArrayBlockStorage<Size>::GetLight(int index) const
{
  return blocks_[index].GetLight();
}






template<unsigned Size>
inline void PaletteBlockStorage<Size>::SetBlock(int index, BlockType type)
{
  pblock_.SetVal(index, type);
}

template<unsigned Size>
inline Block PaletteBlockStorage<Size>::GetBlock(int index) const
{
  return Block(GetBlockType(index), GetLight(index));
}

template<unsigned Size>
inline BlockType PaletteBlockStorage<Size>::GetBlockType(int index) const
{
  return pblock_.GetVal(index);
}

template<unsigned Size>
inline void PaletteBlockStorage<Size>::SetLight(int index, Light light)
{
  plight_.SetVal(index, light);
}

template<unsigned Size>
inline Light PaletteBlockStorage<Size>::GetLight(int index) const
{
  return plight_.GetVal(index);
}