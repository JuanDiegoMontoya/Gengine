#include <Chunks/Chunk.h>
#include <block.h>
//#include "World.h"
#include "chunk_manager.h"
#include <Graphics/utilities.h>
#include <Chunks/ChunkHelpers.h>
#include <Chunks/ChunkStorage.h>
#include <Graphics/GraphicsIncludes.h>
#include <Utilities/Timer.h>

#include <algorithm>
#include <execution>
#include <mutex>


ChunkManager::ChunkManager()
{
  loadDistance_ = 0;
  unloadLeniency_ = 0;
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
  //  ChunkStorage::GetMapRaw().begin(),
  //  ChunkStorage::GetMapRaw().end(),
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
  auto cptr = ChunkStorage::GetChunk(cpos.chunk_pos);
  if (cptr)
  {
    //std::lock_guard<std::mutex> lock(chunk_mesher_mutex_);
    UpdateChunk(cptr);
  }
}


void ChunkManager::UpdateBlock(const glm::ivec3& wpos, Block bl, bool indirect)
{
  ChunkHelpers::localpos p = ChunkHelpers::worldPosToLocalPos(wpos);
  //BlockPtr block = Chunk::AtWorld(wpos);
  Block remBlock = ChunkStorage::AtWorldD(p); // store state of removed block to update lighting
  Chunk* chunk = ChunkStorage::GetChunk(p.chunk_pos);

  // create empty chunk if it's null
  if (!chunk)
  {
    // make chunk, then modify changed block
    ChunkStorage::GetMapRaw()[p.chunk_pos] = chunk = new Chunk(p.chunk_pos);
    remBlock = chunk->BlockAt(p.block_pos); // remBlock would've been 0 block cuz null, so it's fix here
  }

  if (indirect == false)
    chunk->SetBlockTypeAt(p.block_pos, bl.GetType());
  else
    chunk->SetBlockTypeAtIndirect(p.block_pos, bl.GetType());

  // check if removed block emitted light
  lightPropagateRemove(wpos);

  // check if added block emits light
  glm::uvec3 emit2 = Block::PropertiesTable[int(bl.GetType())].emittance;
  if (emit2 != glm::uvec3(0))
    lightPropagateAdd(wpos, Light(Block::PropertiesTable[int(bl.GetType())].emittance));

  // add to update list if it ain't
  UpdateChunk(chunk);

  // check if adjacent to opaque blocks in nearby chunks, then update those chunks if it is
  const glm::ivec3 dirs[] =
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
  ChunkStorage::GetChunk(l.chunk_pos)->SetBlockTypeAt(l.block_pos, block.GetType());
  //*Chunk::AtWorld(wpos) = block;
  //UpdatedChunk(Chunk::chunks[Chunk::worldBlockToLocalPos(wpos).chunk_pos]);
}


void ChunkManager::ReloadAllChunks()
{
  for (const auto& p : ChunkStorage::GetMapRaw())
  {
    if (p.second)
    {
      //std::lock_guard<std::mutex> lock(chunk_mesher_mutex_);
      UpdateChunk(p.second);
    }
      //if (!isChunkInUpdateList(p.second))
      //  updatedChunks_.push_back(p.second);
  }
}


Chunk* ChunkManager::GetChunk(const glm::ivec3& wpos)
{
  auto l = ChunkHelpers::worldPosToLocalPos(wpos);
  return ChunkStorage::GetChunk(l.chunk_pos);
}


#include <cereal/types/vector.hpp>
#include <cereal/types/utility.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/unordered_map.hpp>
#include <fstream>
void ChunkManager::SaveWorld(std::string fname)
{
  std::ofstream of("./resources/Maps/" + fname + ".bin", std::ios::binary);
  cereal::BinaryOutputArchive archive(of);
  std::vector<Chunk*> tempChunks;
  for (const auto& [pos, chunk] : ChunkStorage::GetMapRaw())
  {
    if (chunk)
      tempChunks.push_back(chunk);
  }
  archive(tempChunks);

  std::cout << "Saved to " << fname << "!\n";
}

#pragma optimize("", off)
void ChunkManager::LoadWorld(std::string fname)
{
  for (const auto& [pos, chunk] : ChunkStorage::GetMapRaw())
  {
    delete chunk;
  }
  ChunkStorage::GetMapRaw().clear();

  std::ifstream is("./resources/Maps/" + fname + ".bin", std::ios::binary);
  cereal::BinaryInputArchive archive(is);
  std::vector<Chunk> tempChunks;
  archive(tempChunks);
  for (auto& chunk : tempChunks)
  {
    ChunkStorage::GetMapRaw()[chunk.GetPos()] = new Chunk(chunk);
  }

  ReloadAllChunks();
  std::cout << "Loaded " << fname << "!\n";
}


void ChunkManager::checkUpdateChunkNearBlock(const glm::ivec3& pos, const glm::ivec3& near)
{
  // skip if both blocks are in same chunk
  auto p1 = ChunkHelpers::worldPosToLocalPos(pos);
  auto p2 = ChunkHelpers::worldPosToLocalPos(pos + near);
  if (p1.chunk_pos == p2.chunk_pos)
    return;

  // update chunk if near block is NOT air/invisible
  //BlockPtr cb = Chunk::AtWorld(pos);
  //BlockPtr nb = Chunk::AtWorld(pos + near);
  //if (cb && nb && nb->GetType() != BlockType::bAir)
  Chunk* cptr = ChunkStorage::GetChunk(p2.chunk_pos);
  if (cptr)
  {
    //std::lock_guard<std::mutex> lock(chunk_mesher_mutex_);
    UpdateChunk(cptr);
    delayed_update_queue_.erase(cptr);
  }
    //if (!isChunkInUpdateList(Chunk::chunks[p2.chunk_pos]))
    //  updatedChunks_.push_back(Chunk::chunks[p2.chunk_pos]);
}


// TODO: make this a Safe & Reliable Operation(tm)
//   rather than having it cause crashes often
void ChunkManager::removeFarChunks()
{
  // delete chunks far from the camera (past leniency range)
  if (generation_queue_.size() == 0 && mesher_queue_.size() == 0 && debug_cur_pool_left == 0)
  {
    std::vector<Chunk*> deleteList;
    // attempt at safety
    //std::lock_guard<std::mutex> lock1(chunk_generation_mutex_);
    //std::lock_guard<std::mutex> lock2(chunk_mesher_mutex_);
    //std::lock_guard<std::mutex> lock3(chunk_buffer_mutex_);
    Utils::erase_if(
      ChunkStorage::GetMapRaw(),
      [&](auto& p)->bool
    {
      // range is distance from camera to corner of chunk (corner is ok)
      float dist = glm::distance(glm::vec3(p.first * Chunk::CHUNK_SIZE), Camera::ActiveCamera->GetPos());
      if (p.second && dist > loadDistance_ + unloadLeniency_)
      {
        deleteList.push_back(p.second);
        return true;
      }
      return false;
    });

    for (Chunk* p : deleteList)
      delete p;
  }

  //std::for_each(
  //  std::execution::par,
  //  Chunk::chunks.begin(),
  //  Chunk::chunks.end(),
  //  [&](auto& p)
  //{
  //  float dist = glm::distance(glm::vec3(p.first * Chunk::CHUNK_SIZE), Renderer::GetPipeline()->GetCamera(0)->GetPos());
  //  if (p.second)
  //  {
  //    if (dist > loadDistance_ + unloadLeniency_)
  //      p.second->SetActive(false);
  //    else
  //      p.second->SetActive(true);
  //  }
  //});
}


void ChunkManager::createNearbyChunks()
{
  // generate new chunks that are close to the camera
  std::for_each(
    std::execution::par,
    ChunkStorage::GetMapRaw().begin(),
    ChunkStorage::GetMapRaw().end(),
    [&](auto& p)
  {
    float dist = glm::distance(glm::vec3(p.first * Chunk::CHUNK_SIZE), Camera::ActiveCamera->GetPos());
    // generate null chunks within distance
    if (!p.second && dist <= loadDistance_)
    {
      p.second = new Chunk(p.first);
//      p.second->generate_ = true;
      //std::lock_guard<std::mutex> lock1(chunk_generation_mutex_);
      generation_queue_.insert(p.second);
    }
  });
}

#pragma optimize("", on)
// TODO: make lighting updates also check chunks around the cell 
// (because lighting affects all neighboring blocks)
// ref https://www.seedofandromeda.com/blogs/29-fast-flood-fill-lighting-in-a-blocky-voxel-game-pt-1
// wpos: world position
// nLight: new lighting value
// skipself: chunk updating thing
void ChunkManager::lightPropagateAdd(glm::ivec3 wpos, Light nLight, bool skipself, bool sunlight, bool noqueue)
{
  // get existing light at the position
  auto optL = ChunkStorage::AtWorldE(wpos);
  if (optL.has_value())
  {
    // if there is already light in the spot,
    // combine the two by taking the max values only
    glm::u8vec4 t = glm::max(optL->GetLight().Get(), nLight.Get());
    ChunkStorage::SetLight(wpos, t);
    //L.Set(t); //*L = t;
  }
  
  // queue of world positions, rather than chunk + local index (they are equivalent)
  std::queue<glm::ivec3> lightQueue;
  lightQueue.push(wpos);

  while (!lightQueue.empty())
  {
    glm::ivec3 lightp = lightQueue.front(); // light position
    lightQueue.pop();
    Light lightLevel = ChunkStorage::AtWorldC(lightp).GetLight(); // node that will be giving light to others
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
      auto nblock = ChunkStorage::AtWorldE(nlightPos);
      if (!nblock.has_value()) // skip if block invalid (outside of world)
        continue;
      Light nlight = nblock->GetLight();

      // add chunk to update queue if it exists
      if (!noqueue)
        delayed_update_queue_.insert(ChunkStorage::GetChunk(ChunkHelpers::worldPosToLocalPos(nlightPos).chunk_pos));

      // if neighbor is solid block, skip dat boi
      if (Block::PropertiesTable[nblock->GetTypei()].visibility == Visibility::Opaque)
        continue;

      // iterate over R, G, B
      bool enqueue = false;
      for (int ci = 0; ci < 4; ci++)
      {
        // neighbor must have light level 2 less than current to be updated
        if (nlight.Get()[ci] + 2 > lightLevel.Get()[ci])
          continue;

        // TODO: light propagation through transparent materials
        // get all light systems (R, G, B, Sun) and modify ONE of them,
        // then push the position of that light into the queue
        //glm::u8vec4 val = light.Get();
        // this line can be optimized to reduce amount of global block getting
        glm::u8vec4 val = ChunkStorage::AtWorldC(nlightPos).GetLight().Get();
        val[ci] = (lightLevel.Get()[ci] - 1);// *Block::PropertiesTable[block.GetTypei()].color[ci];

        // if sunlight, max light, and going down, then don't decrease power
        if (ci == 3 && lightLevel.Get()[3] == 0xF && dir == glm::ivec3(0, -1, 0))
          val[3] = 0xF;

        ChunkStorage::SetLight(nlightPos, val);
        enqueue = true;
      }
      if (enqueue) // enqueue if any lighting system changed
        lightQueue.push(nlightPos);
    }
  }

  // do not update this chunk again if it contained the placed light
  if (!noqueue && skipself)
    delayed_update_queue_.erase(ChunkStorage::GetChunk(ChunkHelpers::worldPosToLocalPos(wpos).chunk_pos));
}


void ChunkManager::lightPropagateRemove(glm::ivec3 wpos, bool noqueue)
{
  std::queue<std::pair<glm::ivec3, Light>> lightRemovalQueue;
  Light light = ChunkStorage::AtWorldC(wpos).GetLight();
  lightRemovalQueue.push({ wpos, light });
  ChunkStorage::SetLight(wpos, Light({ 0, 0, 0, light.GetS() }));
  //GetBlockPtr(wpos)->GetLightRef().Set({ 0, 0, 0, light.GetS() });

  std::queue<std::pair<glm::ivec3, Light>> lightReadditionQueue;

  while (!lightRemovalQueue.empty())
  {
    auto [plight, lite] = lightRemovalQueue.front();
    auto lightv = lite.Get(); // current light value
    lightRemovalQueue.pop();

    const glm::ivec3 dirs[] =
    {
      { 1, 0, 0 },
      {-1, 0, 0 },
      { 0, 1, 0 },
      { 0,-1, 0 },
      { 0, 0, 1 },
      { 0, 0,-1 },
    };

    for (const auto& dir : dirs)
    {
      glm::ivec3 blockPos = plight + dir;
      auto optB = ChunkStorage::AtWorldE(blockPos);
      //BlockPtr b = GetBlockPtr(plight + dir);
      if (!optB.has_value())
        continue;
      for (int ci = 0; ci < 3; ci++) // iterate 3 color systems (not sunlight)
      {
        // if the removed block emits light, it needs to be re-propagated
        auto emit = Block::PropertiesTable[optB->GetTypei()].emittance;
        if (emit != glm::u8vec4(0))
          lightReadditionQueue.push({ blockPos, emit });
        Light nearLight = optB->GetLight();
        //Light& nearLight = b->GetLightRef();
        glm::u8vec4 nlightv = nearLight.Get(); // near light value

        // skip updates when light is 0
        // remove light if there is any and if it is weaker than this node's light value
        //if (nlightv[ci] != 0)
        {
          if (nlightv[ci] != 0 && nlightv[ci] == lightv[ci] - 1)
          {
            lightRemovalQueue.push({ blockPos, nearLight });
            if (!noqueue && ChunkStorage::GetChunk(ChunkHelpers::worldPosToLocalPos(blockPos).chunk_pos))
              delayed_update_queue_.insert(ChunkStorage::GetChunk(ChunkHelpers::worldPosToLocalPos(blockPos).chunk_pos));
            auto tmp = nearLight.Get();
            tmp[ci] = 0;
            //nearLight.Set(tmp);
            optB->GetLightRef().Set(tmp);
            ChunkStorage::SetLight(blockPos, tmp);
          }
          // re-propagate near light that is equal to or brighter than this after setting it all to 0
          else if (nlightv[ci] > lightv[ci])
          {
            glm::u8vec4 nue(0);
            nue[ci] = nlightv[ci];
            lightReadditionQueue.push({ blockPos, nue });
          }
        }
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
  delayed_update_queue_.erase(ChunkStorage::GetChunk(ChunkHelpers::worldPosToLocalPos(wpos).chunk_pos));
}


bool ChunkManager::checkDirectSunlight(glm::ivec3 wpos)
{
  auto p = ChunkHelpers::worldPosToLocalPos(wpos);
  Chunk* chunk = ChunkStorage::GetChunk(p.chunk_pos);
  if (!chunk)
    return false;
  Block block = chunk->BlockAt(p.block_pos);

  // find the highest valid chunk
  const glm::ivec3 up(0, 1, 0);
  glm::ivec3 cpos = p.chunk_pos + up;
  Chunk* next = chunk;
  while (next)
  {
    chunk = next;
    cpos += up;
    next = ChunkStorage::GetChunk(cpos);
  }

  // go down until we hit another solid block or this block
  return false;
}

void ChunkManager::initializeSunlight()
{
  Timer timer;

  // find the max chunk height (assumed world has flat top, so no column-local max height needed)
  int maxY = std::numeric_limits<int>::min();
  int minY = std::numeric_limits<int>::max();

  for (const auto& [pos, chunk] : ChunkStorage::GetMapRaw())
  {
    minY = glm::min(minY, pos.y);
    maxY = glm::max(maxY, pos.y);
  }

  // generates initial columns of sunlight in the world
  for (auto [cpos, chunk] : ChunkStorage::GetMapRaw())
  {
    // propagate light only from the highest chunks
    if (cpos.y != maxY || chunk == nullptr)
      continue;

    // for each block on top of the chunk
    for (int x = 0; x < Chunk::CHUNK_SIZE; x++)
    {
      for (int z = 0; z < Chunk::CHUNK_SIZE; z++)
      {
        glm::ivec3 lpos(x, Chunk::CHUNK_SIZE - 1, z);
        Block curBlock = chunk->BlockAt(lpos);
        if (Block::PropertiesTable[curBlock.GetTypei()].visibility == Visibility::Opaque)
          continue;

        Light light = chunk->LightAt(lpos);
        light.SetS(0xF);
        chunk->SetLightAt(lpos, light);
        glm::ivec3 wpos = ChunkHelpers::chunkPosToWorldPos(lpos, cpos);
        lightsToPropagate.push(std::move(wpos));
      }
    }
  }

  while (!lightsToPropagate.empty())
  {
    glm::ivec3 wpos = lightsToPropagate.front();
    lightsToPropagate.pop();
    sunlightPropagateOnce(wpos);
  }

  printf("Sunlight propagation took %f seconds\n", timer.elapsed());
}

// updates the sunlight of a block at a given location
void ChunkManager::sunlightPropagateOnce(const glm::ivec3& wpos)
{
  // do something
  enum { left, right, up, down, front, back }; // +Z = front
  const glm::ivec3 dirs[] =
  {
    { 1, 0, 0 },
    {-1, 0, 0 },
    { 0, 1, 0 },
    { 0,-1, 0 },
    { 0, 0, 1 },
    { 0, 0,-1 },
  };

  Light curLight = ChunkStorage::AtWorldC(wpos).GetLight();

  for (int dir = 0; dir < 6; dir++)
  {
    glm::ivec3 neighborPos = wpos + dirs[dir];
    std::optional<Block> neighbor = ChunkStorage::AtWorldE(neighborPos);
    if (!neighbor)
      continue;

    if (Block::PropertiesTable[neighbor->GetTypei()].visibility == Visibility::Opaque)
      continue;

    Light neighborLight = neighbor->GetLight();

    if (neighborLight.GetS() + 2 > curLight.GetS())
      continue;

    if (curLight.GetS() == 0xF && dir == down)
      neighborLight.SetS(0xF);
    else
      neighborLight.SetS(curLight.GetS() - 1);
    ChunkStorage::SetLight(neighborPos, neighborLight);
    lightsToPropagate.push(neighborPos);
  }
}

//void ChunkManager::initializeSunlight()
//{
//  Timer timer;
//  // find the max chunk height (assumed world has flat top, so no column-local max height needed)
//  int maxY = std::numeric_limits<int>::min();
//  int minY = std::numeric_limits<int>::max();
//
//  for (const auto& [pos, chunk] : ChunkStorage::GetMapRaw())
//  {
//    minY = glm::min(minY, pos.y);
//    maxY = glm::max(maxY, pos.y);
//  }
//
//  std::queue<glm::ivec3> lightLocationsToPropagate;
//
//  std::function <void(int, int, Chunk*)> processColumn = [&](const int x, const int z, Chunk* c)
//  {
//    // each column
//    bool broke = false;
//    for (int y = Chunk::CHUNK_SIZE - 1; y >= 0; y--)
//    {
//      glm::ivec3 lpos{ x, y, z };
//      auto wpos = ChunkHelpers::chunkPosToWorldPos(lpos, c->GetPos());
//
//      // break the moment any block in the column is solid; not exposed to sunlight
//      Block curBlock = ChunkStorage::AtWorldC(wpos);
//      if (Block::PropertiesTable[curBlock.GetTypei()].visibility == Visibility::Opaque)
//      {
//        broke = true;
//        break;
//      }
//
//      Light light = c->LightAt(lpos);
//      light.SetS(0xF); // set sunlight to max
//      c->SetLightAt(lpos, light);
//      lightLocationsToPropagate.push(wpos);
//    }
//
//    // if entire column was illuminated, continue to chunk below
//    if (broke == false && c->GetPos().y > minY)
//    {
//      processColumn(x, z, ChunkStorage::GetChunk(c->GetPos() - glm::ivec3(0, 1, 0)));
//    }
//  };
//
//  // generates initial columns of sunlight in the world
//  for (auto [cpos, chunk] : ChunkStorage::GetMapRaw())
//  {
//    // propagate light only from the highest chunks
//    if (cpos.y != maxY || chunk == nullptr)
//      continue;
//
//    // for each block on top of the chunk
//    for (int x = 0; x < Chunk::CHUNK_SIZE; x++)
//    {
//      for (int z = 0; z < Chunk::CHUNK_SIZE; z++)
//      {
//        processColumn(x, z, chunk);
//      }
//    }
//  }
//
//  while (!lightLocationsToPropagate.empty())
//  {
//    auto wpos = lightLocationsToPropagate.front();
//    lightLocationsToPropagate.pop();
//
//    //sunlightPropagateAdd(wpos, 0xF);
//  }
//
//  printf("Sunlight propagation took %f seconds\n", timer.elapsed());
//}