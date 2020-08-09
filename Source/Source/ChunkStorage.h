#pragma once
#include "ChunkHelpers.h"
#include "chunk.h"

class ChunkStorage
{
public:

	static inline ChunkPtr GetChunk(const glm::ivec3& cpos)
	{
		auto it = chunks_.find(cpos);
		if (it != chunks_.end())
			return it->second;
		return nullptr;
	}

	static inline Block AtWorldC(const glm::ivec3& wpos)
	{
		ChunkHelpers::localpos w = ChunkHelpers::worldPosToLocalPos(wpos);
		Chunk* cnk = chunks_[w.chunk_pos];
		if (cnk)
			return cnk->BlockAt(w.block_pos);
		return Block();
	}

	static inline Block AtWorldD(const ChunkHelpers::localpos& p)
	{
		Chunk* cnk = chunks_[p.chunk_pos];
		if (cnk)
			return cnk->BlockAt(p.block_pos);
		return Block();
	}

	static inline std::optional<Block> AtWorldE(const glm::ivec3& wpos)
	{
		ChunkHelpers::localpos w = ChunkHelpers::worldPosToLocalPos(wpos);
		Chunk* cnk = chunks_[w.chunk_pos];
		if (cnk)
			return cnk->BlockAt(w.block_pos);
		return std::nullopt;
	}

	static inline auto& GetMapRaw()
	{
		return chunks_;
	}

	static inline bool SetBlock(const glm::ivec3& wpos, Block b)
	{
		ChunkHelpers::localpos w = ChunkHelpers::worldPosToLocalPos(wpos);
		Chunk* cnk = chunks_[w.chunk_pos];
		if (cnk)
		{
			cnk->SetBlockTypeAt(w.block_pos, b.GetType());
			cnk->SetLightAt(w.block_pos, b.GetLight());
		}
		return false;
	}

	static inline bool SetBlockType(const glm::ivec3& wpos, BlockType bt)
	{
		ChunkHelpers::localpos w = ChunkHelpers::worldPosToLocalPos(wpos);
		Chunk* cnk = chunks_[w.chunk_pos];
		if (cnk)
		{
			cnk->SetBlockTypeAt(w.block_pos, bt);
		}
		return false;
	}

	static inline bool SetLight(const glm::ivec3& wpos, Light l)
	{
		ChunkHelpers::localpos w = ChunkHelpers::worldPosToLocalPos(wpos);
		Chunk* cnk = chunks_[w.chunk_pos];
		if (cnk)
		{
			cnk->SetLightAt(w.block_pos, l);
		}
		return false;
	}

private:
	static inline Concurrency::concurrent_unordered_map // TODO: make CustomGrow(tm) concurrent map solution for portability
		<glm::ivec3, Chunk*, Utils::ivec3Hash> chunks_;
};