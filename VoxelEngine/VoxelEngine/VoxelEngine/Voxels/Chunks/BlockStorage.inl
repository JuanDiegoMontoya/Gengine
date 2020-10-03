#pragma once
#include <Chunks/BlockStorage.h>

template<unsigned _Size>
inline ArrayBlockStorage<_Size>::ArrayBlockStorage()
{
}

template<unsigned _Size>
inline ArrayBlockStorage<_Size>::~ArrayBlockStorage()
{
}

template<unsigned _Size>
inline ArrayBlockStorage<_Size>::ArrayBlockStorage(const ArrayBlockStorage& other)
{
	*this = other;
}

template<unsigned _Size>
inline ArrayBlockStorage<_Size>& ArrayBlockStorage<_Size>::operator=(const ArrayBlockStorage& other)
{
	this->blocks = other.blocks;
	return *this;
}

template<unsigned _Size>
inline Block& ArrayBlockStorage<_Size>::operator[](int index)
{
	return blocks_[index];
}

template<unsigned _Size>
inline Block& ArrayBlockStorage<_Size>::GetBlockRef(int index)
{
	return blocks_[index];
}

template<unsigned _Size>
inline Block ArrayBlockStorage<_Size>::GetBlock(int index)
{
	return blocks_[index];
}

template<unsigned _Size>
inline BlockType ArrayBlockStorage<_Size>::GetBlockType(int index)
{
	return blocks_[index].GetType();
}

template<unsigned _Size>
inline void ArrayBlockStorage<_Size>::SetBlock(int index, BlockType type)
{
	blocks_[index].SetType(type);
}

template<unsigned _Size>
inline void ArrayBlockStorage<_Size>::SetLight(int index, Light light)
{
	blocks_[index].GetLightRef() = light;
}

template<unsigned _Size>
inline Light ArrayBlockStorage<_Size>::GetLight(int index)
{
	return blocks_[index].GetLight();
}






template<unsigned _Size>
inline void PaletteBlockStorage<_Size>::SetBlock(int index, BlockType type)
{
	pblock_.SetVal(index, type);
}

template<unsigned _Size>
inline Block PaletteBlockStorage<_Size>::GetBlock(int index)
{
	return Block(GetBlockType(index), GetLight(index));
}

template<unsigned _Size>
inline BlockType PaletteBlockStorage<_Size>::GetBlockType(int index)
{
	return pblock_.GetVal(index);
}

template<unsigned _Size>
inline void PaletteBlockStorage<_Size>::SetLight(int index, Light light)
{
	plight_.SetVal(index, light);
}

template<unsigned _Size>
inline Light PaletteBlockStorage<_Size>::GetLight(int index)
{
	return plight_.GetVal(index);
}