//#include "stdafx.h"
#include <Voxels/chunk_manager.h>
#include <algorithm>
#include <execution>
#include <CoreEngine/Camera.h>
#include <Voxels/VoxelManager.h>

// TODO: add a way to notify these threads to terminate when the program
// does to prevent crash on exit

namespace Utils
{
  struct ChunkPtrKeyEq
  {
    bool operator()(const Chunk* first, const Chunk* second) const
    {
      //ASSERT(first != second);
      if (first == second)
        return false;
      glm::vec3 wposA = glm::vec3(first->GetPos() * Chunk::CHUNK_SIZE);
      glm::vec3 wposB = glm::vec3(second->GetPos() * Chunk::CHUNK_SIZE);
      glm::vec3 cam = Camera::ActiveCamera->GetPos();
      return
        glm::distance(wposA, cam) <
        glm::distance(wposB, cam);
    }
  };
}

// perpetual thread task to generate blocks in new chunks
void ChunkManager::chunk_generator_thread_task()
{
  //while (!shutdownThreads)
  //{
  //  //std::set<Chunk*, Utils::Chunk*KeyEq> temp;
  //  std::unordered_set<Chunk*> temp;
  //  {
  //    std::lock_guard<std::mutex> lock1(chunk_generation_mutex_);
  //    temp.swap(generation_queue_);
  //  }

  //  std::for_each(std::execution::seq, temp.begin(), temp.end(), [this](Chunk* chunk)
  //  {
  //    //WorldGen::GenerateChunk(chunk->GetPos());
  //    std::lock_guard<std::mutex> lock2(chunk_mesher_mutex_);
  //    mesher_queue_.insert(chunk);
  //  });

  //  //std::lock_guard<std::mutex> lock2(chunk_mesher_mutex_);
  //  //mesher_queue_.insert(temp.begin(), temp.end());
  //}
}


// TODO: figure how to make this parallel again (if necessary)
// perpetual thread task to generate meshes for updated chunks
void ChunkManager::chunk_mesher_thread_task()
{
  while (!shutdownThreads)
  {
    //std::set<Chunk*, Utils::Chunk*KeyEq> temp;
    //std::set<Chunk*> temp;
    Concurrency::concurrent_unordered_set<Chunk*> temp;
    std::vector<Chunk*> sorted; // to-be ordered set containing temp's items
    {
      //std::lock_guard<std::mutex> lock1(chunk_mesher_mutex_);
      std::lock_guard<std::mutex> lock1(t_mesher_mutex_);
      temp.swap(t_mesher_queue_);
      debug_cur_pool_left += temp.size();
    }
    sorted.insert(sorted.begin(), temp.begin(), temp.end());

    // TODO: this is temp solution to load near chunks to camera first
    std::sort(std::execution::par_unseq, sorted.begin(), sorted.end(), Utils::ChunkPtrKeyEq());
    std::for_each(std::execution::seq, sorted.begin(), sorted.end(), [this](Chunk* chunk)
    {
      //SetThreadAffinityMask(GetCurrentThread(), ~1);
      // send each mesh to GPU immediately after building it
      chunk->BuildMesh();
      debug_cur_pool_left--;
      //std::lock_guard<std::mutex> lock2(chunk_buffer_mutex_);
      buffer_queue_.insert(chunk);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(0));
  }
  //std::shared_ptr<void> fdsa;
  //fdsa.use_count();
}


void ChunkManager::chunk_deferred_update_task()
{
  while (!shutdownThreads)
  {
    //std::for_each(std::execution::seq, ChunkStorage::GetMapRaw().begin(), ChunkStorage::GetMapRaw().end()
    //[]()
    for (auto [pos, chunk] : voxelManager.chunks_)
    {
      
    }
  }
}


// sends vertex data of fully-updated chunks to GPU from main thread (fast and simple)
void ChunkManager::chunk_buffer_task()
{
  //std::set<Chunk*, Utils::Chunk*KeyEq> temp;
  Concurrency::concurrent_unordered_set<Chunk*> temp;
  {
    //std::lock_guard<std::mutex> lock(chunk_buffer_mutex_);
    temp.swap(buffer_queue_);
  }

  // normally, there will only be a few items in here per frame
  for (Chunk* chunk : temp)
    chunk->BuildBuffers();
}


// copy-pasted from above functions
void ChunkManager::chunk_gen_mesh_nobuffer()
{
  //{
  //  std::unordered_set<Chunk*> temp;
  //  {
  //    std::lock_guard<std::mutex> lock1(chunk_generation_mutex_);
  //    temp.swap(generation_queue_);
  //  }

  //  std::for_each(std::execution::seq, temp.begin(), temp.end(), [this](Chunk* chunk)
  //    {
  //      //WorldGen::GenerateChunk(chunk->GetPos());
  //      std::lock_guard<std::mutex> lock2(chunk_mesher_mutex_);
  //      mesher_queue_.insert(chunk);
  //    });
  //}

  //{
  //  std::unordered_set<Chunk*> temp;
  //  std::vector<Chunk*> sorted; // to-be ordered set containing temp's items
  //  {
  //    std::lock_guard<std::mutex> lock1(chunk_mesher_mutex_);
  //    temp.swap(mesher_queue_);
  //    debug_cur_pool_left += temp.size();
  //  }
  //  sorted.insert(sorted.begin(), temp.begin(), temp.end());

  //  // TODO: this is temp solution to load near chunks to camera first
  //  std::sort(sorted.begin(), sorted.end(), Utils::Chunk*KeyEq());
  //  std::for_each(std::execution::seq, sorted.begin(), sorted.end(), [this](Chunk* chunk)
  //  {
  //    //SetThreadAffinityMask(GetCurrentThread(), ~1);
  //    // send each mesh to GPU immediately after building it
  //    chunk->BuildMesh();
  //    debug_cur_pool_left--;
  //    std::lock_guard<std::mutex> lock2(chunk_buffer_mutex_);
  //    buffer_queue_.insert(chunk);
  //  });
  //}
}