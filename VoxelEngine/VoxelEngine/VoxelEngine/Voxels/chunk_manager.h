#pragma once
#include <Voxels/Chunk.h>
#include <Voxels/block.h>

#include <set>
//#include <unordered_set>
#pragma warning(disable : 4996)
#define _SILENCE_CXX17_OLD_ALLOCATOR_MEMBERS_DEPRECATION_WARNING
#include <concurrent_unordered_set.h>
#include <atomic>
#include <stack>
#include <queue>

#include <CoreEngine/Camera.h>
#include <ctpl_stl.h>

template<typename T>
class AtomicQueue
{
public:
  AtomicQueue() {};

  void Push(const T& val)
  {
    std::lock_guard lck(mutex_);
    queue_.push(val);
  }

  T Pop()
  {
    std::lock_guard lck(mutex_);
    T val = queue_.front();
    queue_.pop();
    return val;
  }

  // single lock, low-overhead
  template<typename Callback>
  void ForEach(Callback fn, unsigned maxIterations = UINT32_MAX)
  {
    if (maxIterations == 0)
    {
      maxIterations = UINT32_MAX;
    }

    std::lock_guard lck(mutex_);
    while (!queue_.empty() && maxIterations != 0)
    {
      T val = queue_.front();
      queue_.pop();
      fn(val);
      maxIterations--;
    }
  }

private:
  std::queue<T> queue_;
  mutable std::shared_mutex mutex_;
};

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