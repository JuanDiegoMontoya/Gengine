#pragma once
#include <Voxels/Chunk.h>
#include <Voxels/block.h>
#include <Utilities/AtomicQueue.h>
#include <ctpl_stl.h>

namespace Voxels
{
  class VoxelManager;

  // Interfaces with the Chunk class to
  // manage how and when chunk block and mesh data is generated, and
  // when that data is sent to the GPU.
  // Also manages updates to blocks and lighting in chunks to determine
  // when a chunk needs to be remeshed.
  class ChunkManager
  {
  public:
    ChunkManager(VoxelManager& manager);
    ~ChunkManager() {};
    void Init();
    void Destroy();

    // interaction
    void Update();
    void UpdateChunk(Chunk* chunk);
    void UpdateChunk(const glm::ivec3& wpos); // update chunk at block position
    void UpdateBlock(const glm::ivec3& wpos, Block bl);
    void UpdateBlockCheap(const glm::ivec3& wpos, Block block);
    void ReloadAllChunks(); // for when big things change


    // TODO: move
    //void SaveWorld(std::string fname);
    //void LoadWorld(std::string fname);

  private:
    // functions
    void checkUpdateChunkNearBlock(const glm::ivec3& pos, const glm::ivec3& near);

    //AtomicQueue<Chunk*> mesherQueueGood_;
    ctpl::thread_pool mesherThreadPool_;
    AtomicQueue<Chunk*> bufferQueueGood_;

    // new light intensity to add
    std::vector<Chunk*> lightPropagateAdd(const glm::ivec3& wpos, Light nLight);
    std::vector<Chunk*> lightPropagateRemove(const glm::ivec3& wpos);

    VoxelManager& voxelManager;
  };
}