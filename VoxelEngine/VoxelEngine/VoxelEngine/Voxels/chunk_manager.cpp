#include <Voxels/Chunk.h>
#include <Voxels/block.h>
//#include "World.h"
#include "chunk_manager.h"
#include <CoreEngine/utilities.h>
#include <Voxels/ChunkHelpers.h>
#include <CoreEngine/GraphicsIncludes.h>
#include <Utilities/Timer.h>
#include "VoxelManager.h"

#include <algorithm>
#include <execution>
#include <mutex>


ChunkManager::ChunkManager(VoxelManager& manager) 
  : voxelManager(manager)
{
  debug_cur_pool_left = 0;
}


ChunkManager::~ChunkManager()
{
  shutdownThreads = true;
  for (auto& t_ptr : chunk_generator_threads_)
  {
    t_ptr->join();
  }
  for (auto& t_ptr : chunk_mesher_threads_)
  {
    t_ptr->join();
  }
}


void ChunkManager::Init()
{
  // run main thread on core 1
  //SetThreadAffinityMask(GetCurrentThread(), 1);
  // spawn chunk block generator threads
  //for (int i = 0; i < 4; i++)
  //{
  //  chunk_generator_threads_.push_back(
  //    new std::thread([this]() { chunk_generator_thread_task(); }));
  //  //SetThreadAffinityMask(chunk_generator_threads_[i]->native_handle(), ~1);
  //}

  // spawn chunk mesh generator threads
  for (int i = 0; i < 1; i++)
  {
    chunk_mesher_threads_.push_back(
      std::make_unique<std::thread>([this]() { chunk_mesher_thread_task(); }));
    //SetThreadAffinityMask(chunk_mesher_threads_[i]->native_handle(), ~1);
  }
}

void ChunkManager::Update()
{
  //PERF_BENCHMARK_START;

  // TODO: update a random selection of the chunks per frame

  //std::for_each(
  //  std::execution::par,
  //  voxelManager.chunks_.begin(),
  //  voxelManager.chunks_.end(),
  //  [](auto& p)
  //{
  //  if (p.second)
  //    p.second->Update();
  //});

  chunk_buffer_task();
  //chunk_gen_mesh_nobuffer();
  if (mesher_queue_.size() > 0)
  {
    std::lock_guard<std::mutex> lock1(t_mesher_mutex_);
    mesher_queue_.swap(t_mesher_queue_);
    //mesher_queue_.clear();
  }

  for (Chunk* chunk : delayed_update_queue_)
    UpdateChunk(chunk);
  delayed_update_queue_.clear();

  //PERF_BENCHMARK_END;
}


void ChunkManager::UpdateChunk(Chunk* chunk)
{
  ASSERT(chunk != nullptr);
  //std::lock_guard<std::mutex> lock(chunk_mesher_mutex_);
  mesher_queue_.insert(chunk);
}


void ChunkManager::UpdateChunk(const glm::ivec3 wpos)
{
  auto cpos = ChunkHelpers::worldPosToLocalPos(wpos);
  //auto cptr = Chunk::chunks[cpos.chunk_pos];
  auto cptr = voxelManager.GetChunk(cpos.chunk_pos);
  if (cptr)
  {
    //std::lock_guard<std::mutex> lock(chunk_mesher_mutex_);
    UpdateChunk(cptr);
  }
}


void ChunkManager::UpdateBlock(const glm::ivec3& wpos, Block bl)
{
  ChunkHelpers::localpos p = ChunkHelpers::worldPosToLocalPos(wpos);
  //BlockPtr block = Chunk::AtWorld(wpos);
  Block remBlock = voxelManager.GetBlock(p); // store state of removed block to update lighting
  Chunk* chunk = voxelManager.GetChunk(p.chunk_pos);

  // create empty chunk if it's null
  if (!chunk)
  {
    // make chunk, then modify changed block
    voxelManager.chunks_[voxelManager.flatten(p.chunk_pos)] = chunk = new Chunk(p.chunk_pos, voxelManager);
    remBlock = chunk->BlockAt(p.block_pos); // remBlock would've been 0 block cuz null, so it's fix here
  }

  chunk->SetBlockTypeAt(p.block_pos, bl.GetType());

  // check if removed block emitted light
  if (bl.GetVisibility() == Visibility::Opaque || remBlock.GetVisibility() == Visibility::Opaque)
  {
    lightPropagateRemove(wpos);
  }

  // propagate sunlight down from above, if applicable
  //if (auto block = voxelManager.TryGetBlock(wpos + glm::ivec3(0, 1, 0)); block && block->GetLight().GetS() > 1)
  //{
  //  lightPropagateAdd(wpos + glm::ivec3(0, 1, 0), block->GetLight());
  //}
  
  // check if added block emits light
  if (glm::uvec3 emit2 = bl.GetEmittance(); emit2 != glm::uvec3(0))
  {
    lightPropagateAdd(wpos, Light(Block::PropertiesTable[bl.GetTypei()].emittance));
  }

  // add to update list if it ain't
  UpdateChunk(chunk);

  // check if adjacent to opaque blocks in nearby chunks, then update those chunks if it is
  constexpr glm::ivec3 dirs[] =
  {
    {-1, 0, 0 },
    { 1, 0, 0 },
    { 0,-1, 0 },
    { 0, 1, 0 },
    { 0, 0,-1 },
    { 0, 0, 1 }
    // TODO: add 20 more cases for diagonals (AO)
  };
  for (const auto& dir : dirs)
  {
    checkUpdateChunkNearBlock(wpos, dir);
  }

  delayed_update_queue_.erase(chunk);
}


// perform no checks, therefore the chunk must be known prior to placing the block
void ChunkManager::UpdateBlockCheap(const glm::ivec3& wpos, Block block)
{
  auto l = ChunkHelpers::worldPosToLocalPos(wpos);
  voxelManager.GetChunk(l.chunk_pos)->SetBlockTypeAt(l.block_pos, block.GetType());
  //*Chunk::AtWorld(wpos) = block;
  //UpdatedChunk(Chunk::chunks[Chunk::worldBlockToLocalPos(wpos).chunk_pos]);
}


void ChunkManager::ReloadAllChunks()
{
  for (const auto& p : voxelManager.chunks_)
  {
    if (p)
    {
      //std::lock_guard<std::mutex> lock(chunk_mesher_mutex_);
      UpdateChunk(p);
    }
      //if (!isChunkInUpdateList(p.second))
      //  updatedChunks_.push_back(p.second);
  }
}


Chunk* ChunkManager::GetChunk(const glm::ivec3& wpos)
{
  auto l = ChunkHelpers::worldPosToLocalPos(wpos);
  return voxelManager.GetChunk(l.chunk_pos);
}


//#include <cereal/types/vector.hpp>
//#include <cereal/types/utility.hpp>
//#include <cereal/archives/binary.hpp>
//#include <cereal/types/unordered_map.hpp>
//#include <fstream>
//void ChunkManager::SaveWorld(std::string fname)
//{
//  std::ofstream of("./resources/Maps/" + fname + ".bin", std::ios::binary);
//  cereal::BinaryOutputArchive archive(of);
//  std::vector<Chunk*> tempChunks;
//  for (const auto& [pos, chunk] : voxelManager.chunks_)
//  {
//    if (chunk)
//      tempChunks.push_back(chunk);
//  }
//  archive(tempChunks);
//
//  std::cout << "Saved to " << fname << "!\n";
//}
//
//void ChunkManager::LoadWorld(std::string fname)
//{
//  for (const auto& [pos, chunk] : voxelManager.chunks_)
//  {
//    delete chunk;
//  }
//  voxelManager.chunks_.clear();
//
//  std::ifstream is("./resources/Maps/" + fname + ".bin", std::ios::binary);
//  cereal::BinaryInputArchive archive(is);
//  std::vector<Chunk> tempChunks;
//  archive(tempChunks);
//  for (auto& chunk : tempChunks)
//  {
//    voxelManager.chunks_[chunk.GetPos()] = new Chunk(chunk);
//  }
//
//  ReloadAllChunks();
//  std::cout << "Loaded " << fname << "!\n";
//}


void ChunkManager::checkUpdateChunkNearBlock(const glm::ivec3& pos, const glm::ivec3& near)
{
  // skip if both blocks are in same chunk
  auto p1 = ChunkHelpers::worldPosToLocalPos(pos);
  auto p2 = ChunkHelpers::worldPosToLocalPos(pos + near);
  if (p1.chunk_pos == p2.chunk_pos)
  {
    return;
  }

  // update chunk if near block is NOT air/invisible
  //BlockPtr cb = Chunk::AtWorld(pos);
  //BlockPtr nb = Chunk::AtWorld(pos + near);
  //if (cb && nb && nb->GetType() != BlockType::bAir)
  Chunk* cptr = voxelManager.GetChunk(p2.chunk_pos);
  if (cptr)
  {
    //std::lock_guard<std::mutex> lock(chunk_mesher_mutex_);
    UpdateChunk(cptr);
    delayed_update_queue_.erase(cptr);
  }
    //if (!isChunkInUpdateList(Chunk::chunks[p2.chunk_pos]))
    //  updatedChunks_.push_back(Chunk::chunks[p2.chunk_pos]);
}


// TODO: make lighting updates also check chunks around the cell 
// (because lighting affects all neighboring blocks)
// ref https://www.seedofandromeda.com/blogs/29-fast-flood-fill-lighting-in-a-blocky-voxel-game-pt-1
// wpos: world position
// nLight: new lighting value
// skipself: chunk updating thing
void ChunkManager::lightPropagateAdd(glm::ivec3 wpos, Light nLight, bool skipself, bool sunlight, bool noqueue)
{
  // get existing light at the position
  auto optL = voxelManager.TryGetBlock(wpos);
  if (optL.has_value())
  {
    // if there is already light in the spot,
    // combine the two by taking the max values only
    glm::u8vec4 t = glm::max(optL->GetLight().Get(), nLight.Get());
    voxelManager.SetBlockLight(wpos, t);
    //L.Set(t); //*L = t;
  }
  
  // queue of world positions, rather than chunk + local index (they are equivalent)
  std::queue<glm::ivec3> lightQueue;
  lightQueue.push(wpos);

  while (!lightQueue.empty())
  {
    glm::ivec3 lightp = lightQueue.front(); // light position
    lightQueue.pop();
    Light lightLevel = voxelManager.GetBlock(lightp).GetLight(); // node that will be giving light to others
    const glm::ivec3 dirs[] =
    {
      { 1, 0, 0 },
      {-1, 0, 0 },
      { 0, 1, 0 },
      { 0,-1, 0 },
      { 0, 0, 1 },
      { 0, 0,-1 },
    };

    // update each neighbor
    for (const auto& dir : dirs)
    {
      glm::ivec3 nlightPos = lightp + dir;
      auto nblock = voxelManager.TryGetBlock(nlightPos);
      if (!nblock) continue;
      Light nlight = nblock->GetLight();

      // add chunk to update queue if it exists
      if (!noqueue)
      {
        delayed_update_queue_.insert(voxelManager.GetChunk(ChunkHelpers::worldPosToLocalPos(nlightPos).chunk_pos));
      }

      // if neighbor is solid block, skip dat boi
      if (Block::PropertiesTable[nblock->GetTypei()].visibility == Visibility::Opaque)
      {
        continue;
      }

      // iterate over R, G, B, Sun
      bool enqueue = false;
      for (int ci = 0; ci < 4; ci++)
      {
        // neighbor must have light level 2 or less than current to be updated
        // AND isn't sunlight going down (in which case it can update any lights lesser in strength)
        if (nlight.Get()[ci] + 2 > lightLevel.Get()[ci] && !(ci == 3 && nlight.Get()[3] + 1 == lightLevel.Get()[3] && dir == glm::ivec3(0, -1, 0)))
        //if (nlight.Get()[ci] + 2 > lightLevel.Get()[ci])
        {
          continue;
        }

        // TODO: light propagation through transparent materials
        // get all light systems (R, G, B, Sun) and modify ONE of them,
        // then push the position of that light into the queue
        //glm::u8vec4 val = light.Get();
        // this line can be optimized to reduce amount of global block getting
        glm::u8vec4 val = voxelManager.GetBlock(nlightPos).GetLight().Get();
        val[ci] = (lightLevel.Get()[ci] - 1);// *Block::PropertiesTable[block.GetTypei()].color[ci];

        // if sunlight, max light, and going down, then don't decrease power
        if (ci == 3 && lightLevel.Get()[3] == 0xF && dir == glm::ivec3(0, -1, 0))
        {
          val[3] = 0xF;
        }

        voxelManager.SetBlockLight(nlightPos, val);
        enqueue = true;
      }
      if (enqueue) // enqueue if any lighting system changed
      {
        lightQueue.push(nlightPos);
      }
    }
  }

  // do not update this chunk again if it contained the placed light
  if (!noqueue && skipself)
  {
    delayed_update_queue_.erase(voxelManager.GetChunk(ChunkHelpers::worldPosToLocalPos(wpos).chunk_pos));
  }
}


void ChunkManager::lightPropagateRemove(glm::ivec3 wpos, bool noqueue)
{
  std::queue<std::pair<glm::ivec3, Light>> lightRemovalQueue;
  Block wblock = voxelManager.GetBlock(wpos);
  Light light = wblock.GetLight();
  lightRemovalQueue.push({ wpos, light });

  //if (wblock.GetVisibility() == Visibility::Opaque || wblock.GetEmittance())
  {
    voxelManager.SetBlockLight(wpos, Light({ 0, 0, 0, 0 }));
  }

  std::queue<std::pair<glm::ivec3, Light>> lightReadditionQueue;

  const glm::ivec3 dirs[] = 
    { { 1, 0, 0 },
      {-1, 0, 0 },
      { 0, 1, 0 },
      { 0,-1, 0 },
      { 0, 0, 1 },
      { 0, 0,-1 } };

  while (!lightRemovalQueue.empty())
  {
    const auto [plight, lite] = lightRemovalQueue.front();
    const auto lightv = lite.Get(); // current light value
    lightRemovalQueue.pop();

    for (const auto& dir : dirs)
    {
      glm::ivec3 blockPos = plight + dir;
      auto optB = voxelManager.TryGetBlock(blockPos);
      if (!optB) continue;

      const Light nearLight = optB->GetLight();
      glm::u8vec4 nlightv = nearLight.Get();
      glm::u8vec4 nue(0);
      bool enqueueRemove = false;
      bool enqueueReadd = false;
      for (int ci = 0; ci < 4; ci++) // iterate 4 colors (including sunlight)
      {
        // remove light if there is any and if it is weaker than this node's light value, OR if max sunlight and going down
        if (nlightv[ci] > 0 && ((nlightv[ci] == lightv[ci] - 1)) || (ci == 3 && dir == glm::ivec3(0, -1, 0) && nlightv[3] == 0xF))
        {
          enqueueRemove = true;
          nlightv[ci] = 0;
          voxelManager.SetBlockLight(blockPos, nlightv);
        }
        // re-propagate near light that is equal to or brighter than this after setting it all to 0
        // OR if it is sunlight of any strength, NOT down from this position
        else if (nlightv[ci] > lightv[ci] || (ci == 3 && nlightv[3] > 0 && dir != glm::ivec3(0, -1, 0)))
        {
          enqueueReadd = true;
          nue[ci] = nlightv[ci];
        }
      }
      
      if (enqueueRemove)
      {
        lightRemovalQueue.push({ blockPos, nearLight });
        if (!noqueue && voxelManager.GetChunk(ChunkHelpers::worldPosToLocalPos(blockPos).chunk_pos))
        {
          delayed_update_queue_.insert(voxelManager.GetChunk(ChunkHelpers::worldPosToLocalPos(blockPos).chunk_pos));
        }
      }
      if (enqueueReadd)
      {
        lightReadditionQueue.push({ blockPos, nue });
      }
    }
  }


  // re-propogate lights in queue, otherwise we're left with hard edges
  while (!lightReadditionQueue.empty())
  {
    const auto& [pos, light] = lightReadditionQueue.front();
    lightReadditionQueue.pop();
    lightPropagateAdd(pos, light, false);
  }

  // do not update the removed block's chunk again since the act of removing will update it
  delayed_update_queue_.erase(voxelManager.GetChunk(ChunkHelpers::worldPosToLocalPos(wpos).chunk_pos));
}