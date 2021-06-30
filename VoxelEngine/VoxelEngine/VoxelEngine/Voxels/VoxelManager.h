#pragma once
#include <memory>
#include <Voxels/Chunk.h>
#include <Voxels/ChunkHelpers.h>

class Editor;
class Scene;
class WorldGen;

namespace Voxels
{
  class ChunkManager;
  class ChunkRenderer;

  class VoxelManager
  {
  public:
    VoxelManager(Scene& scene);
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
    Chunk* GetChunkNoCheck(const glm::ivec3& cpos);
    const Chunk* GetChunkNoCheck(const glm::ivec3& cpos) const;
    std::vector<Chunk*> GetChunksRegion(const glm::ivec3& lowCpos, const glm::ivec3& highCpos);
    std::vector<Chunk*> GetChunksRegionWorldSpace(const glm::ivec3& lowWpos, const glm::ivec3& highWpos);
    Block GetBlock(const glm::ivec3& wpos) const;
    Block GetBlock(const ChunkHelpers::localpos& p) const;
    std::optional<Block> TryGetBlock(const glm::ivec3& wpos) const;

    // Meshing-aware block-changing functions. Expensive, but convenient.
    // Call these functions infrequently, such as during common gameplay actions.
    void UpdateBlock(const glm::ivec3& wpos, Block block);
    void UpdateBlockCheap(const glm::ivec3& wpos, Block block);

    // Change the state of the voxel world. These functions are cheap to call as 
    // they only modify the block data, but do not cause the chunk to be remeshed.
    // Call these functions for data-heavy work such as world generation.
    bool SetBlock(const glm::ivec3& wpos, Block block);
    bool SetBlockType(const glm::ivec3& wpos, BlockType type);
    bool SetBlockLight(const glm::ivec3& wpos, Light light);

    // Regenerates the mesh of the chunk at the specified location
    void UpdateChunk(const glm::ivec3& cpos);
    void UpdateChunk(Chunk* chunk);

    // Utility functions
    void Raycast(glm::vec3 origin, glm::vec3 direction, float distance, std::function<bool(glm::vec3, Block, glm::vec3)> callback);

    // TODO: privatize once a way to set settings is added
    std::unique_ptr<ChunkRenderer> chunkRenderer_{};

  private:
    friend class ChunkManager;
    friend class WorldGen;
    friend class ChunkMesh;
    friend class Editor;

    uint32_t flatten(glm::ivec3 p) const
    {
      return p.x + actualWorldDim_.x * (p.y + actualWorldDim_.y * p.z);
    }
    Chunk* find(const glm::ivec3& p) const
    {
      if (glm::all(glm::lessThan(p, actualWorldDim_)) &&
        glm::all(glm::greaterThanEqual(p, glm::ivec3(0))))
      {
        return chunks_[flatten(p)];
      }
      return nullptr;
    }
    Chunk* findNoCheck(const glm::ivec3& p) const
    {
      ASSERT(glm::all(glm::lessThan(p, actualWorldDim_)) &&
        glm::all(glm::greaterThanEqual(p, glm::ivec3(0))));
      return chunks_[flatten(p)];
    }


    std::unique_ptr<ChunkManager> chunkManager_{};
    std::vector<Chunk*> chunks_;
    glm::ivec3 virtualWorldDim_{ 0, 0, 0 };
    glm::ivec3 actualWorldDim_{ 0, 0, 0 };
    glm::ivec3 chunksPerDim_{};

    std::unique_ptr<Editor> editor_{};
    Scene& scene_;
  };



  inline void VoxelManager::SetDim(const glm::ivec3& newDim)
  {
    ASSERT(chunks_.size() == 0);

    virtualWorldDim_ = newDim;
    actualWorldDim_ = newDim + 2;
    chunks_.resize(actualWorldDim_.x * actualWorldDim_.y * actualWorldDim_.z, nullptr);

    for (int i = 0; i < chunks_.size(); i++)
    {
      glm::ivec3 cpos
      {
        i % actualWorldDim_.x,
        (i / actualWorldDim_.x) % actualWorldDim_.y,
        i / (actualWorldDim_.y * actualWorldDim_.x),
      };
      if (glm::all(glm::lessThanEqual(cpos, virtualWorldDim_)) &&
        glm::all(glm::greaterThan(cpos, glm::ivec3(0))))
      {
        Chunk* newChunk = new Chunk(cpos, *this);
        chunks_[flatten(cpos)] = newChunk;
      }
    }
  }

  inline Chunk* VoxelManager::GetChunk(const glm::ivec3& cpos)
  {
    return find(cpos);
  }

  inline const Chunk* VoxelManager::GetChunk(const glm::ivec3& cpos) const
  {
    return find(cpos);
  }

  inline Chunk* VoxelManager::GetChunkNoCheck(const glm::ivec3& cpos)
  {
    return findNoCheck(cpos);
  }

  inline const Chunk* VoxelManager::GetChunkNoCheck(const glm::ivec3& cpos) const
  {
    return findNoCheck(cpos);
  }

  inline std::vector<Chunk*> VoxelManager::GetChunksRegion(const glm::ivec3& lowCpos, const glm::ivec3& highCpos)
  {
    auto low = glm::min(lowCpos, highCpos);
    auto high = glm::max(lowCpos, highCpos);
    std::vector<Chunk*> region;
    region.reserve((high.x - low.x + 1) * (high.y - low.y + 1) * (high.z - low.z + 1));
    for (int z = low.z; z <= high.z; z++)
    {
      for (int y = low.y; y <= high.y; y++)
      {
        for (int x = low.x; x <= high.x; x++)
        {
          if (auto found = find({ x, y, z }))
          {
            region.push_back(found);
          }
        }
      }
    }
    return region;
  }

  inline std::vector<Chunk*> VoxelManager::GetChunksRegionWorldSpace(const glm::ivec3& lowWpos, const glm::ivec3& highWpos)
  {
    auto lowCpos = ChunkHelpers::WorldPosToLocalPos(lowWpos).chunk_pos;
    auto highCpos = ChunkHelpers::WorldPosToLocalPos(highWpos).chunk_pos;
    return GetChunksRegion(lowCpos, highCpos);
  }

  inline Block VoxelManager::GetBlock(const glm::ivec3& wpos) const
  {
    ChunkHelpers::localpos wp = ChunkHelpers::WorldPosToLocalPos(wpos);
    return chunks_[flatten(wp.chunk_pos)]->BlockAt(wp.block_pos);
  }

  inline Block VoxelManager::GetBlock(const ChunkHelpers::localpos& wp) const
  {
    return chunks_[flatten(wp.chunk_pos)]->BlockAt(wp.block_pos);
  }

  inline std::optional<Block> VoxelManager::TryGetBlock(const glm::ivec3& wpos) const
  {
    ChunkHelpers::localpos wp = ChunkHelpers::WorldPosToLocalPos(wpos);
    Chunk* chunk = find(wp.chunk_pos);
    if (chunk)
      return chunk->BlockAt(wp.block_pos);
    return std::nullopt;
  }

  inline bool VoxelManager::SetBlock(const glm::ivec3& wpos, Block block)
  {
    ChunkHelpers::localpos w = ChunkHelpers::WorldPosToLocalPos(wpos);
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
    ChunkHelpers::localpos w = ChunkHelpers::WorldPosToLocalPos(wpos);
    Chunk* chunk = chunks_[flatten(w.chunk_pos)];
    if (chunk)
    {
      chunk->SetBlockTypeAt(w.block_pos, type);
      return true;
    }
    return false;
  }

  inline bool VoxelManager::SetBlockLight(const glm::ivec3& wpos, Light light)
  {
    ChunkHelpers::localpos w = ChunkHelpers::WorldPosToLocalPos(wpos);
    Chunk* chunk = chunks_[flatten(w.chunk_pos)];
    if (chunk)
    {
      chunk->SetLightAt(w.block_pos, light);
      return true;
    }
    return false;
  }
}