#pragma once
#include <Chunks/Chunk.h>
#include <block.h>

#include <set>
//#include <unordered_set>
#pragma warning(disable : 4996)
#define _SILENCE_CXX17_OLD_ALLOCATOR_MEMBERS_DEPRECATION_WARNING
#include <concurrent_unordered_set.h>
#include <atomic>
#include <stack>
#include <queue>

#include <Camera.h>

struct Chunk;
//class ChunkLoadManager;



// Interfaces with the Chunk class to
// manage how and when chunk block and mesh data is generated, and
// when that data is sent to the GPU.
// Also manages updates to blocks and lighting in chunks to determine
// when a chunk needs to be remeshed.

// TODO: make this an abstract class, then send its current contents to a
// new class called "InfiniteChunkManager" or something like that,
// then create another called "FixedChunkManager", both derived from
// this class.
class ChunkManager
{
public:
  ChunkManager();
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

  // getters
  float GetLoadDistance() const { return loadDistance_; }
  float GetUnloadLeniency() const { return unloadLeniency_; }

  // setters
  //void SetCurrentLevel(LevelPtr level) { level_ = level; }
  void SetLoadDistance(float d) { loadDistance_ = d; }
  void SetUnloadLeniency(float d) { unloadLeniency_ = d; }

  void SaveWorld(std::string fname);
  void LoadWorld(std::string fname);

  friend class Level; // so level can display debug info
private:
public: // TODO: TEMPORARY
  // functions
  void checkUpdateChunkNearBlock(const glm::ivec3& pos, const glm::ivec3& near);

  void removeFarChunks();
  void createNearbyChunks();

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
  bool checkDirectSunlight(glm::ivec3 wpos);
  void initializeSunlight();
  void sunlightPropagateOnce(const glm::ivec3& wpos);
  std::queue<glm::ivec3> lightsToPropagate;

  // vars
  float loadDistance_;
  float unloadLeniency_;
  bool shutdownThreads = false;
  //std::vector<Chunk*> updatedChunks_;
  //std::vector<Chunk*> genChunkList_;
  //LevelPtr level_ = nullptr;
};