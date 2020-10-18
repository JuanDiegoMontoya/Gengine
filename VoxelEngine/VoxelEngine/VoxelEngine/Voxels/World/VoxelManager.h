#pragma once
#include <memory>
#include <concurrent_unordered_map.h>
#include <Chunks/Chunk.h>
#include <Chunks/ChunkHelpers.h>

class ChunkManager;

class VoxelManager
{
public:
  VoxelManager();
  ~VoxelManager();

  VoxelManager(VoxelManager&) = delete;
  VoxelManager(VoxelManager&&) noexcept = delete;

  VoxelManager& operator=(const VoxelManager&) = delete;
  VoxelManager& operator=(VoxelManager&&) = delete;

  // Get information about the voxel world
  Chunk* GetChunk(const glm::ivec3& cpos);
  Block GetBlock(const glm::ivec3& wpos);
  Block GetBlock(const ChunkHelpers::localpos& p);
  std::optional<Block> TryGetBlock(const glm::ivec3& wpos);

  // Change the state of the voxel world
  bool SetBlock(const glm::ivec3& wpos, Block block);
  bool SetBlockType(const glm::ivec3& wpos, BlockType type);
  bool SetLight(const glm::ivec3& wpos, Light light);

private:
  friend class ChunkManager;
  //friend class WorldGen;

  std::unique_ptr<ChunkManager> chunkManager_{};
  Concurrency::concurrent_unordered_map<glm::ivec3, Chunk*, Utils::ivec3Hash> chunks_{};
};







Chunk* VoxelManager::GetChunk(const glm::ivec3& cpos)
{
  auto it = chunks_.find(cpos);
  if (it != chunks_.end())
    return it->second;
  return nullptr;
}

Block VoxelManager::GetBlock(const glm::ivec3& wpos)
{
  ChunkHelpers::localpos w = ChunkHelpers::worldPosToLocalPos(wpos);
  Chunk* chunk = chunks_[w.chunk_pos];
  ASSERT(chunk);
  return chunk->BlockAt(w.block_pos);
}

Block VoxelManager::GetBlock(const ChunkHelpers::localpos& p)
{
  Chunk* chunk = chunks_[p.chunk_pos];
  ASSERT(chunk);
  return chunk->BlockAt(p.block_pos);
}

std::optional<Block> VoxelManager::TryGetBlock(const glm::ivec3& wpos)
{
  ChunkHelpers::localpos w = ChunkHelpers::worldPosToLocalPos(wpos);
  Chunk* chunk = chunks_[w.chunk_pos];
  if (chunk)
    return chunk->BlockAt(w.block_pos);
  return std::nullopt;
}

bool VoxelManager::SetBlock(const glm::ivec3& wpos, Block block)
{
  ChunkHelpers::localpos w = ChunkHelpers::worldPosToLocalPos(wpos);
  Chunk* chunk = chunks_[w.chunk_pos];
  if (chunk)
  {
    chunk->SetBlockTypeAt(w.block_pos, block.GetType());
    chunk->SetLightAt(w.block_pos, block.GetLight());
    return true;
  }
  return false;
}

bool VoxelManager::SetBlockType(const glm::ivec3& wpos, BlockType type)
{
  ChunkHelpers::localpos w = ChunkHelpers::worldPosToLocalPos(wpos);
  Chunk* chunk = chunks_[w.chunk_pos];
  if (chunk)
  {
    chunk->SetBlockTypeAt(w.block_pos, type);
    return true;
  }
  return false;
}

bool VoxelManager::SetLight(const glm::ivec3& wpos, Light light)
{
  ChunkHelpers::localpos w = ChunkHelpers::worldPosToLocalPos(wpos);
  Chunk* chunk = chunks_[w.chunk_pos];
  if (chunk)
  {
    chunk->SetLightAt(w.block_pos, light);
    return true;
  }
  return false;
}
