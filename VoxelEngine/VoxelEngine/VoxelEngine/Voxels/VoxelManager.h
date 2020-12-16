#pragma once
#include <memory>
#include <concurrent_unordered_map.h>
#include <Voxels/Chunk.h>
#include <Voxels/ChunkHelpers.h>

class ChunkManager;
class ChunkRenderer;

inline uint32_t Part1By2(uint32_t x)
{
  x &= 0x000003ff;                  // x = ---- ---- ---- ---- ---- --98 7654 3210
  x = (x ^ (x << 16)) & 0xff0000ff; // x = ---- --98 ---- ---- ---- ---- 7654 3210
  x = (x ^ (x << 8)) & 0x0300f00f; // x = ---- --98 ---- ---- 7654 ---- ---- 3210
  x = (x ^ (x << 4)) & 0x030c30c3; // x = ---- --98 ---- 76-- --54 ---- 32-- --10
  x = (x ^ (x << 2)) & 0x09249249; // x = ---- 9--8 --7- -6-- 5--4 --3- -2-- 1--0
  return x;
}
inline uint32_t EncodeMorton3(const glm::ivec3& p)
{
  ASSERT(glm::all(glm::lessThan(p, glm::ivec3(2 << 10))) && glm::all(glm::greaterThanEqual(p, glm::ivec3(0))));
  return (Part1By2(p.z) << 2) + (Part1By2(p.y) << 1) + Part1By2(p.x);
}


class VoxelManager
{
public:
  VoxelManager();
  ~VoxelManager();

  VoxelManager(VoxelManager&) = delete;
  VoxelManager(VoxelManager&&) noexcept = delete;

  VoxelManager& operator=(const VoxelManager&) = delete;
  VoxelManager& operator=(VoxelManager&&) = delete;

  void SetDim(const glm::ivec3& newDim);

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
  friend class WorldGen;
  friend class ChunkMesh;

  std::unique_ptr<ChunkManager> chunkManager_{};
  //Concurrency::concurrent_unordered_map<glm::ivec3, Chunk*, Utils::ivec3Hash> chunks_{};
  std::vector<Chunk*> chunks_;
  glm::ivec3 worldDim_{ 0, 0, 0 };
  glm::ivec3 chunksPerDim_{};
  uint32_t flatten(const glm::ivec3& p) const
  {
    //ASSERT(glm::all(glm::lessThan(p, worldDim_)) && glm::all(glm::greaterThanEqual(p, glm::ivec3(0))));
    //return glm::dot(glm::vec3(p), glm::vec3(chunksPerDim_));
    return p.x + worldDim_.x * (p.y + worldDim_.y * p.z);
  }
  Chunk* find(const glm::ivec3& p) const
  {
    if (glm::all(glm::lessThan(p, worldDim_)) && glm::all(glm::greaterThanEqual(p, glm::ivec3(0))))
      return chunks_[flatten(p)];
    return nullptr;
  }
};



inline void VoxelManager::SetDim(const glm::ivec3& newDim)
{
  ASSERT(chunks_.size() == 0);
  worldDim_ = newDim;
  chunksPerDim_ = { 1, worldDim_.x, worldDim_.x * worldDim_.y };
  chunks_.resize(newDim.x * newDim.y * newDim.z);
  //int m = glm::ceil(glm::log2((float)glm::max(newDim.x, glm::max(newDim.y, newDim.z))));
  //ASSERT(m <= 10);
  //chunks_.resize((1 << (3 * m)));
}

inline Chunk* VoxelManager::GetChunk(const glm::ivec3& cpos)
{
  //auto it = chunks_.find(cpos);
  //if (it != chunks_.end())
  //  return it->second;
  //return nullptr;
  return find(cpos);
}

inline const Chunk* VoxelManager::GetChunk(const glm::ivec3& cpos) const
{
  //auto it = chunks_.find(cpos);
  //if (it != chunks_.cend())
  //  return it->second;
  //return nullptr;
  return find(cpos);
}

inline Block VoxelManager::GetBlock(const glm::ivec3& wpos) const
{
  ChunkHelpers::localpos wp = ChunkHelpers::worldPosToLocalPos(wpos);
  //auto it = chunks_.find(wp.chunk_pos);
  //ASSERT(it != chunks_.cend());
  //return it->second->BlockAt(wp.block_pos);
  return chunks_[flatten(wp.chunk_pos)]->BlockAt(wp.block_pos);
}

inline Block VoxelManager::GetBlock(const ChunkHelpers::localpos& wp) const
{
  //auto it = chunks_.find(wp.chunk_pos);
  //ASSERT(it != chunks_.cend());
  //return it->second->BlockAt(wp.block_pos);
  return chunks_[flatten(wp.chunk_pos)]->BlockAt(wp.block_pos);
}

inline std::optional<Block> VoxelManager::TryGetBlock(const glm::ivec3& wpos) const
{
  ChunkHelpers::localpos wp = ChunkHelpers::worldPosToLocalPos(wpos);
  //auto it = chunks_.find(wp.chunk_pos);
  //if (it != chunks_.end())
  //  return it->second->BlockAt(wp.block_pos);
  //return std::nullopt;
  Chunk* chunk = find(wp.chunk_pos);
  if (chunk)
    return chunk->BlockAt(wp.block_pos);
  return std::nullopt;
}

inline bool VoxelManager::SetBlock(const glm::ivec3& wpos, Block block)
{
  ChunkHelpers::localpos w = ChunkHelpers::worldPosToLocalPos(wpos);
  Chunk* chunk = chunks_[flatten(w.chunk_pos)];
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
  Chunk* chunk = chunks_[flatten(w.chunk_pos)];
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
  Chunk* chunk = chunks_[flatten(w.chunk_pos)];
  if (chunk)
  {
    chunk->SetLightAt(w.block_pos, light);
    return true;
  }
  return false;
}
