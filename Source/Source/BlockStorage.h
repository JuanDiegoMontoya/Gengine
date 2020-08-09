#pragma once
#include "block.h"
#include "BitArray.h"
#include "Palette.h"
#include <array>

// uncompressed block storage for chunks
template<unsigned _Size>
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
	Block GetBlock(int index);
	BlockType GetBlockType(int index);
	void SetBlock(int index, BlockType);
	void SetLight(int index, Light);
	Light GetLight(int index);

private:

	std::array<Block, _Size> blocks_;
	//Block* blocks_ = nullptr;
};


// https://www.reddit.com/r/VoxelGameDev/comments/9yu8qy/palettebased_compression_for_chunked_discrete/
// compressed block storage
// can't really return references w/o doing crazy proxy class stuff
template<unsigned _Size>
class PaletteBlockStorage
{
public:
	void SetBlock(int index, BlockType);
	Block GetBlock(int index);
	BlockType GetBlockType(int index);
	void SetLight(int index, Light);
	Light GetLight(int index);

	PaletteBlockStorage& operator=(const PaletteBlockStorage& other)
	{
		pblock_ = other.pblock_;
		plight_ = other.plight_;
		return *this;
	}

private:
	friend class cereal::access;

	template <class Archive>
	void serialize(Archive& ar)
	{
		ar(pblock_);
	}

	ConcurrentPalette<BlockType, _Size> pblock_;
	ConcurrentPalette<Light, _Size> plight_;
};

#include "BlockStorage.inl"