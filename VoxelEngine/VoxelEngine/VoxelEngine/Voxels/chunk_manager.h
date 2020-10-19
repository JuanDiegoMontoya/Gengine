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

class VoxelManager;
//class ChunkLoadManager;



// Interfaces with the Chunk class to
// manage how and when chunk block and mesh data is generated, and
// when that data is sent to the GPU.
// Also manages updates to blocks and lighting in chunks to determine
// when a chunk needs to be remeshed.
class ChunkManager
{
public:
  ChunkManager(VoxelManager& manager);
  ~ChunkManager();
  void Init();

  // interaction
  void Update();
  void UpdateChunk(Chunk* chunk);
  void UpdateChunk(const glm::ivec3 wpos); // update chunk at block position
  void UpdateBlock(const glm::ivec3& wpos, Block bl, bool indirect = false);
  //void UpdateBlockIndirect(const glm::ivec3& wpos, Block block);
  void UpdateBlockCheap(const glm::ivec3& wpos, Block block);
  void ReloadAllChunks(); // for when big things change

  Chunk* GetChunk(const glm::ivec3& wpos);


  // TODO: move
  //void SaveWorld(std::string fname);
  //void LoadWorld(std::string fname);

private:
public: // TODO: TEMPORARY
  // functions
  void checkUpdateChunkNearBlock(const glm::ivec3& pos, const glm::ivec3& near);

  void chunk_deferred_update_task();

  // generates actual blocks
  void chunk_generator_thread_task();
  //std::set<Chunk*, Utils::Chunk*KeyEq> generation_queue_;
  Concurrency::concurrent_unordered_set<Chunk*> generation_queue_;
  //std::mutex chunk_generation_mutex_;
  std::vector<std::unique_ptr<std::thread>> chunk_generator_threads_;

  // generates meshes for ANY UPDATED chunk
  void chunk_mesher_thread_task();
  //std::set<Chunk*, Utils::Chunk*KeyEq> mesher_queue_;
  //std::set<Chunk*> mesher_queue_;
  Concurrency::concurrent_unordered_set<Chunk*> mesher_queue_;
  //std::mutex chunk_mesher_mutex_;
  std::vector<std::unique_ptr<std::thread>> chunk_mesher_threads_;
  std::atomic_int debug_cur_pool_left = 0;

  std::vector<std::unique_ptr<std::thread>> indirect_update_queue_;

  // NOT multithreaded task
  void chunk_buffer_task();
  //std::set<Chunk*, Utils::Chunk*KeyEq> buffer_queue_;
  Concurrency::concurrent_unordered_set<Chunk*> buffer_queue_;
  //std::mutex chunk_buffer_mutex_;

  std::mutex t_mesher_mutex_;
  Concurrency::concurrent_unordered_set<Chunk*> t_mesher_queue_;

  // DEBUG does everything in a serial fashion
  // chunk_buffer_task must be called after this
  void chunk_gen_mesh_nobuffer();

  std::unordered_set<Chunk*> delayed_update_queue_;

  // new light intensity to add
  void lightPropagateAdd(glm::ivec3 wpos, Light nLight, bool skipself = true, bool sunlight = false, bool noqueue = false);
  void lightPropagateRemove(glm::ivec3 wpos, bool noqueue = false);


  // SUNLIGHT STUFF
  
  // returns true if block at max sunlight level

  // vars
  bool shutdownThreads = false;
  //std::vector<Chunk*> updatedChunks_;
  //std::vector<Chunk*> genChunkList_;

  VoxelManager& voxelManager;
};