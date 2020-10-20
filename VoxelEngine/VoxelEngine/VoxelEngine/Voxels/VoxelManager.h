#pragma once
#include <memory>
#include <concurrent_unordered_map.h>
#include <Voxels/Chunk.h>
#include <Voxels/ChunkHelpers.h>

class ChunkManager;
class ChunkRenderer;

class VoxelManager
{
public:
  VoxelManager();
  ~VoxelManager();

  VoxelManager(VoxelManager&) = delete;
  VoxelManager(VoxelManager&&) noexcept = delete;

  VoxelManager& operator=(const VoxelManager&) = delete;
  VoxelManager& operator=(VoxelManager&&) = delete;

  // Should be called regularly to ensure chunks are continuously meshed
  void Update();
  void Draw();

  // Get information about the voxel world
  Chunk* GetChunk(const glm::ivec3& cpos);
  const Chunk* GetChunk(const glm::ivec3& cpos) const;
  Block GetBlock(const glm::ivec3& wpos) const;
  Block GetBlock(const ChunkHelpers::localpos& p) const;
  std::optional<Block> TryGetBlock(const glm::ivec3& wpos) const;

  // Meshing-aware block-changing functions
  void UpdateBlock(const glm::ivec3& wpos, Block block);
  void UpdateBlockCheap(const glm::ivec3& wpos, Block block);

  // Change the state of the voxel world
  bool SetBlock(const glm::ivec3& wpos, Block block);
  bool SetBlockType(const glm::ivec3& wpos, BlockType type);
  bool SetLight(const glm::ivec3& wpos, Light light);
  void MeshChunk(const glm::ivec3& cpos);

  // Utility functions
  void Raycast(glm::vec3 origin, glm::vec3 direction, float distance, std::function<bool(glm::vec3, Block, glm::vec3)> callback);

  // TODO: make private after NuRenderer finally dies
  std::unique_ptr<ChunkRenderer> chunkRenderer_{};
private:
  friend class ChunkManager;
  friend class WorldGen2;
  friend class ChunkMesh;

  std::unique_ptr<ChunkManager> chunkManager_{};
  Concurrency::concurrent_unordered_map<glm::ivec3, Chunk*, Utils::ivec3Hash> chunks_{};
};







inline Chunk* VoxelManager::GetChunk(const glm::ivec3& cpos)
{
  auto it = chunks_.find(cpos);
  if (it != chunks_.end())
    return it->second;
  return nullptr;
}

inline const Chunk* VoxelManager::GetChunk(const glm::ivec3& cpos) const
{
  auto it = chunks_.find(cpos);
  if (it != chunks_.cend())
    return it->second;
  return nullptr;
}

inline Block VoxelManager::GetBlock(const glm::ivec3& wpos) const
{
  ChunkHelpers::localpos wp = ChunkHelpers::worldPosToLocalPos(wpos);
  auto it = chunks_.find(wp.chunk_pos);
  ASSERT(it != chunks_.cend());
  return it->second->BlockAt(wp.block_pos);
}

inline Block VoxelManager::GetBlock(const ChunkHelpers::localpos& wp) const
{
  auto it = chunks_.find(wp.chunk_pos);
  ASSERT(it != chunks_.cend());
  return it->second->BlockAt(wp.block_pos);
}

inline std::optional<Block> VoxelManager::TryGetBlock(const glm::ivec3& wpos) const
{
  ChunkHelpers::localpos w = ChunkHelpers::worldPosToLocalPos(wpos);
  auto it = chunks_.find(w.chunk_pos);
  if (it != chunks_.cend())
    return it->second->BlockAt(w.block_pos);
  return std::nullopt;
}

inline bool VoxelManager::SetBlock(const glm::ivec3& wpos, Block block)
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

inline bool VoxelManager::SetBlockType(const glm::ivec3& wpos, BlockType type)
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

inline bool VoxelManager::SetLight(const glm::ivec3& wpos, Light light)
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
