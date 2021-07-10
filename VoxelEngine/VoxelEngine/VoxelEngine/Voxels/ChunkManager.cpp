#include "vPCH.h"
#include <Voxels/Chunk.h>
#include <Voxels/block.h>
//#include "World.h"
#include "ChunkManager.h"
#include "ChunkHelpers.h"
#include <CoreEngine/utilities.h>

#include <CoreEngine/GraphicsIncludes.h>
#include <Utilities/Timer.h>
#include "VoxelManager.h"

#include <algorithm>
#include <execution>
#include <mutex>

namespace Voxels
{
  ChunkManager::ChunkManager(VoxelManager& manager)
    : voxelManager(manager)
  {
  }


  void ChunkManager::Destroy()
  {
    mesherThreadPool_.stop(false);
  }


  void ChunkManager::Init()
  {
    mesherThreadPool_.resize(8);
  }


  void ChunkManager::Update()
  {
    bufferQueueGood_.ForEach([](Chunk* chunk) { chunk->BuildBuffers(); }, 0);
  }


  void ChunkManager::UpdateChunk(Chunk* chunk)
  {
    ASSERT(chunk != nullptr);
    mesherThreadPool_.push([chunk, this](int)
      {
        chunk->BuildMesh();
        bufferQueueGood_.Push(chunk);
      });
  }


  void ChunkManager::UpdateChunk(const glm::ivec3& wpos)
  {
    auto cpos = ChunkHelpers::WorldPosToLocalPos(wpos);
    auto cptr = voxelManager.GetChunk(cpos.chunk_pos);
    if (cptr)
    {
      UpdateChunk(cptr);
    }
  }


  void ChunkManager::UpdateBlock(const glm::ivec3& wpos, Block bl)
  {
    ChunkHelpers::localpos p = ChunkHelpers::WorldPosToLocalPos(wpos);
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

    // all chunks that were modified by a propagated lighting change
    std::vector<Chunk*> lightModifiedSet;

    // check if removed block emitted light
    if (bl.GetVisibility() == Visibility::Opaque || remBlock.GetVisibility() == Visibility::Opaque)
    {
      auto lightRemovedSet = lightPropagateRemove(wpos);
      lightModifiedSet.insert(std::end(lightModifiedSet), std::begin(lightRemovedSet), std::end(lightRemovedSet));
    }

    // propagate sunlight down from above, if applicable
    //if (auto block = voxelManager.TryGetBlock(wpos + glm::ivec3(0, 1, 0)); block && block->GetLight().GetS() > 1)
    //{
    //  lightPropagateAdd(wpos + glm::ivec3(0, 1, 0), block->GetLight());
    //}

    // check if added block emits light
    if (glm::uvec3 emit2 = bl.GetEmittance(); emit2 != glm::uvec3(0))
    {
      auto lightAddedSet = lightPropagateAdd(wpos, Light(Block::PropertiesTable[bl.GetTypei()].emittance));
      lightModifiedSet.insert(std::end(lightModifiedSet), std::begin(lightAddedSet), std::end(lightAddedSet));
    }

    // add to update list if it ain't
    UpdateChunk(chunk);

    std::sort(std::begin(lightModifiedSet), std::end(lightModifiedSet));
    lightModifiedSet.erase(std::unique(std::begin(lightModifiedSet), std::end(lightModifiedSet)), std::end(lightModifiedSet));
    printf("Updating %d chunks\n", (int)lightModifiedSet.size());
    for (auto mchunk : lightModifiedSet)
    {
      UpdateChunk(mchunk);
    }

    // check if adjacent to opaque blocks in nearby chunks, then update those chunks if it is
    constexpr glm::ivec3 dirs[] =
    {
      {-1, 0, 0 },
      { 1, 0, 0 },
      { 0,-1, 0 },
      { 0, 1, 0 },
      { 0, 0,-1 },
      { 0, 0, 1 }
      // TODO: add 20 more cases for diagonals (if AO is enabled)
    };
    for (const auto& dir : dirs)
    {
      checkUpdateChunkNearBlock(wpos, dir);
    }
  }


  // perform no checks, therefore the chunk must be known prior to placing the block
  void ChunkManager::UpdateBlockCheap(const glm::ivec3& wpos, Block block)
  {
    auto l = ChunkHelpers::WorldPosToLocalPos(wpos);
    voxelManager.GetChunk(l.chunk_pos)->SetBlockTypeAtNoLock(l.block_pos, block.GetType());
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
    auto p1 = ChunkHelpers::WorldPosToLocalPos(pos);
    auto p2 = ChunkHelpers::WorldPosToLocalPos(pos + near);
    if (p1.chunk_pos == p2.chunk_pos)
    {
      return;
    }

    // update chunk if near block is NOT air/invisible
    Chunk* cptr = voxelManager.GetChunk(p2.chunk_pos);
    if (cptr)
    {
      UpdateChunk(cptr);
    }
  }

  // TODO: lock all chunks in a column, because sunlight can propagate infinitely far down
  // (because lighting affects all neighboring blocks)
  // ref https://www.seedofandromeda.com/blogs/29-fast-flood-fill-lighting-in-a-blocky-voxel-game-pt-1
  // wpos: world position
  // nLight: new lighting value
  // skipself: chunk updating thing
  std::vector<Chunk*> ChunkManager::lightPropagateAdd(const glm::ivec3& wpos, Light nLight)
  {
    // lock all the surrounding chunks, which may be written to (we want to write to all of them in one atomic operation)
    auto potentiallyModifiedSet = voxelManager.GetChunksRegionWorldSpace(wpos - Chunk::CHUNK_SIZE, wpos + Chunk::CHUNK_SIZE);
    std::vector<Chunk*> definitelyModifiedSet;

    for (auto chunk : potentiallyModifiedSet)
    {
      chunk->Lock();
    }

    // get existing light at the position
    auto posLocal = ChunkHelpers::WorldPosToLocalPos(wpos);
    auto chunk = voxelManager.GetChunk(posLocal.chunk_pos);

    if (chunk)
    {
      // if there is already light in the spot,
      // combine the two by taking the max values only
      glm::u8vec4 t = glm::max(chunk->LightAtNoLock(posLocal.block_pos).Get(), nLight.Get());
      chunk->SetLightAtNoLock(posLocal.block_pos, Light(t));
    }

    // queue of world positions, rather than chunk + local index (they are equivalent)
    std::queue<glm::ivec3> lightQueue;
    lightQueue.push(wpos);

    while (!lightQueue.empty())
    {
      glm::ivec3 lightp = lightQueue.front(); // light position
      lightQueue.pop();
      auto lightPosLocal = ChunkHelpers::WorldPosToLocalPos(lightp);
      auto lchunk = voxelManager.GetChunk(lightPosLocal.chunk_pos);
      Light lightLevel = lchunk->LightAtNoLock(lightPosLocal.block_pos);

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
        auto nlightPosLocal = ChunkHelpers::WorldPosToLocalPos(nlightPos);
        auto nchunk = voxelManager.GetChunk(nlightPosLocal.chunk_pos);
        if (!nchunk) continue;
        Block nblock = nchunk->BlockAtNoLock(nlightPosLocal.block_pos);
        Light nlight = nblock.GetLight();

        //auto nblock = voxelManager.TryGetBlock(nlightPos);
        //if (!nblock) continue;
        //Light nlight = nblock->GetLight();

        // if neighbor is solid block, skip dat boi
        if (Block::PropertiesTable[nblock.GetTypei()].visibility == Visibility::Opaque)
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
          {
            continue;
          }

          // TODO: light propagation through transparent materials
          // get all light systems (R, G, B, Sun) and modify ONE of them,
          // then push the position of that light into the queue
          //glm::u8vec4 val = light.Get();
          // this line can be optimized to reduce amount of global block getting
          glm::u8vec4 val = nchunk->LightAtNoLock(nlightPosLocal.block_pos).Get();
          val[ci] = (lightLevel.Get()[ci] - 1);

          // if sunlight, max light, and going down, then don't decrease power
          if (ci == 3 && lightLevel.Get()[3] == 0xF && dir == glm::ivec3(0, -1, 0))
          {
            val[3] = 0xF;
          }

          nchunk->SetLightAtNoLock(nlightPosLocal.block_pos, val);
          definitelyModifiedSet.push_back(nchunk);
          enqueue = true;
        }

        if (enqueue) // enqueue if any lighting system changed
        {
          lightQueue.push(nlightPos);
        }
      }
    }

    for (auto modifiedChunk : potentiallyModifiedSet)
    {
      modifiedChunk->Unlock();
    }

    return definitelyModifiedSet;
  }


  std::vector<Chunk*> ChunkManager::lightPropagateRemove(const glm::ivec3& wpos)
  {
    auto potentiallyModifiedSet = voxelManager.GetChunksRegionWorldSpace(wpos - Chunk::CHUNK_SIZE, wpos + Chunk::CHUNK_SIZE);
    std::vector<Chunk*> definitelyModifiedSet;
    for (auto chunk : potentiallyModifiedSet)
    {
      chunk->Lock();
    }

    std::queue<std::pair<glm::ivec3, Light>> lightRemovalQueue;

    auto posLocal = ChunkHelpers::WorldPosToLocalPos(wpos);
    auto chunk = voxelManager.GetChunkNoCheck(posLocal.chunk_pos);
    Block wblock = chunk->BlockAtNoLock(posLocal.block_pos);
    Light light = wblock.GetLight();

    lightRemovalQueue.push({ wpos, light });

    //if (wblock.GetVisibility() == Visibility::Opaque || wblock.GetEmittance())
    {
      chunk->SetLightAtNoLock(posLocal.block_pos, Light({ 0, 0, 0, 0 }));
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

      auto plightLocalPos = ChunkHelpers::WorldPosToLocalPos(plight);
      //auto pchunk = voxelManager.GetChunkNoCheck(plightLocalPos.chunk_pos);

      for (const auto& dir : dirs)
      {
        glm::ivec3 blockPos = plight + dir;
        auto nlightLocalPos = ChunkHelpers::WorldPosToLocalPos(blockPos);
        auto nchunk = voxelManager.GetChunkNoCheck(nlightLocalPos.chunk_pos);
        if (!nchunk) continue;

        const Light nearLight = nchunk->LightAtNoLock(nlightLocalPos.block_pos);
        glm::u8vec4 nlightv = nearLight.Get();
        glm::u8vec4 nue(0);
        bool enqueueRemove = false;
        bool enqueueReAdd = false;
        for (int ci = 0; ci < 4; ci++) // iterate 4 colors (including sunlight)
        {
          // remove light if there is any and if it is weaker than this node's light value, OR if max sunlight and going down
          if (nlightv[ci] > 0 && ((nlightv[ci] == lightv[ci] - 1)) || (ci == 3 && dir == glm::ivec3(0, -1, 0) && nlightv[3] == 0xF))
          {
            enqueueRemove = true;
            nlightv[ci] = 0;
            nchunk->SetLightAtNoLock(nlightLocalPos.block_pos, nlightv);
            definitelyModifiedSet.push_back(nchunk);
          }
          // re-propagate near light that is equal to or brighter than this after setting it all to 0
          // OR if it is sunlight of any strength, NOT down from this position
          else if (nlightv[ci] > lightv[ci] || (ci == 3 && nlightv[3] > 0 && dir != glm::ivec3(0, -1, 0)))
          {
            enqueueReAdd = true;
            nue[ci] = nlightv[ci];
          }
        }

        if (enqueueRemove)
        {
          lightRemovalQueue.push({ blockPos, nearLight });
        }
        if (enqueueReAdd)
        {
          lightReadditionQueue.push({ blockPos, nue });
        }
      }
    }

    for (auto modifiedChunk : potentiallyModifiedSet)
    {
      modifiedChunk->Unlock();
    }

    // re-propogate lights in queue, otherwise we're left with hard edges
    while (!lightReadditionQueue.empty())
    {
      const auto& [pos, nextLight] = lightReadditionQueue.front();
      lightReadditionQueue.pop();
      std::vector<Chunk*> readdedSet = lightPropagateAdd(pos, nextLight);
      definitelyModifiedSet.insert(std::end(definitelyModifiedSet), std::begin(readdedSet), std::end(readdedSet));
    }

    return definitelyModifiedSet;
  }
}