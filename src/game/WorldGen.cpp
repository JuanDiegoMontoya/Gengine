#include "gPCH.h"
#include "WorldGen.h"
#include <voxel/Chunk.h>
#include <voxel/ChunkHelpers.h>
#include <execution>
#include <utility/Timer.h>
#include <voxel/VoxelManager.h>
#include <voxel/prefab.h>
#include <FastNoise2/include/FastNoise/FastNoise.h>
#include <voxel/ChunkManager.h>
#include <engine/utilities.h>

using namespace Voxels;

namespace
{
#if 0
  glm::ivec3 worldDim{ 70, 4, 70 };
#else
  glm::ivec3 worldDim{ 5, 1, 5 };
#endif
}

// init chunks that we finna modify
void WorldGen::Init()
{
  Timer timer;
  voxels.SetDim(worldDim);
  printf("Allocating chunks took %f seconds\n", timer.Elapsed());
}

// does the thing
void WorldGen::GenerateWorld()
{
  Timer timer;
  //std::unique_ptr<FastNoiseSIMD> noisey(FastNoiseSIMD::NewFastNoiseSIMD());
  //noisey->SetFractalLacunarity(2.0);
  //noisey->SetFractalOctaves(5);
  //noisey->SetSeed(7);
  //noisey->SetFrequency(.04);
  //noisey->SetPerturbType(FastNoiseSIMD::Gradient);
  //noisey->SetPerturbAmp(0.4);
  //noisey->SetPerturbFrequency(0.4);

  //FastNoise::SmartNode<> fnGenerator = FastNoise::NewFromEncodedNodeTree("GgAUAMP1KD8NAAQAAAAAAFBACQAAmpmZPgEEAAAAAAAAAJBBAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
  //FastNoise::SmartNode<> fnGenerator = FastNoise::NewFromEncodedNodeTree("IgAIABIAAgAAADMzE0ARAAAAAEAaABQAw/UoPw0ABAAAAAAAIEAJAAAAAAA/AQQAAAAAAFyPOkEAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAzcxMPgBxPQo/AQcA");
  //FastNoise::SmartNode<> fnGenerator = FastNoise::NewFromEncodedNodeTree("GgAUAMP1KD8NAAQAAAAAAFBACQAAmpmZPgEEAAAAAAAAAJBBAAAAAAAAAAAAAAAACtcjvQAAAAAAAAAA");
  //FastNoise::SmartNode<> fnGenerator = FastNoise::NewFromEncodedNodeTree("DQAFAAAAAAAAQAgAAAAAAD8=");
  FastNoise::SmartNode<> fnGenerator = FastNoise::NewFromEncodedNodeTree("FADD9Sg/DQAEAAAAAAAgQAkAAAAAAD8=");


  auto& chunks = voxels.chunks_;
  std::for_each(std::execution::par, chunks.begin(), chunks.end(),
    [&](Voxels::Chunk* chunk)
  {
    if (chunk)
    {
      const glm::ivec3 cpos = chunk->GetPos();
      glm::ivec3 st = cpos * Voxels::Chunk::CHUNK_SIZE;
      std::array<float, Voxels::Chunk::CHUNK_SIZE_SQRED> noiseSet;
      //noiseSet = noisey->GetCubicFractalSet(st.z, 0, st.x, Voxels::Chunk::CHUNK_SIZE, 1, Voxels::Chunk::CHUNK_SIZE, 1);
      fnGenerator->GenUniformGrid2D(noiseSet.data(), st.x, st.z, Voxels::Chunk::CHUNK_SIZE, Voxels::Chunk::CHUNK_SIZE, .02, 1337);
      /*float* riverNoiseSet = noisey->GetPerlinSet(st.z, 0, st.x,
        Voxels::Chunk::CHUNK_SIZE, 1, Voxels::Chunk::CHUNK_SIZE, 1);*/
      //int idx = 0;

      //printf(".");
      glm::ivec3 pos, wpos;
      int idx = 0;
      for (pos.z = 0; pos.z < Voxels::Chunk::CHUNK_SIZE; pos.z++)
      {
        for (pos.x = 0; pos.x < Voxels::Chunk::CHUNK_SIZE; pos.x++)
        {
          int height = (int)((noiseSet[idx++] + .1f) * 10) + 33;
          for (pos.y = 0; pos.y < Voxels::Chunk::CHUNK_SIZE; pos.y++)
          {
            wpos = ChunkHelpers::LocalPosToWorldPos(pos, cpos);
            int waterHeight = 34;

            //double density = noise.GetValue(wpos.x, wpos.y, wpos.z); // chunks are different
            //double density = noise.GetValue(pos.x, pos.y, pos.z); // same chunk every time
            //density = 0;

            //if (pos.z < 8 && pos.x < 8 && pos.y < 8)
            //  voxels.SetBlockType(wpos, BlockType::bStone);
            //continue;

            //float height = (noiseSet[idx] + .1f) * 10;
            //float density = noiseSet[idx++];
            /*if (density < .0)
            {
              voxels.SetBlockType(wpos, BlockType::bGrass);
            }*/
            if (wpos.y < height)
            {
                voxels.SetBlockType(wpos, BlockType::bGrass);
                if (Utils::noise(pos) < .05f)
                    voxels.SetBlockType(wpos, BlockType::bDirt);
            }
            if (wpos.y < height && wpos.y >= (height - 1))
            {
              voxels.SetBlockType(wpos, BlockType::bGrass);
              if (Utils::noise(pos) < .01f)
              {
                voxels.SetBlockType(wpos, BlockType::bDirt);
                if (height > waterHeight)
                {
                  Prefab prefab = PrefabManager::GetPrefab("OakTree");
                  glm::ivec3 offset = { 0, 1, 0 };
                  for (unsigned i = 0; i < prefab.blocks.size(); i++)
                  {
                    if (auto block = voxels.TryGetBlock(wpos + offset + prefab.blocks[i].first))
                      if (prefab.blocks[i].second.GetPriority() >= block->GetPriority())
                        voxels.SetBlockType(wpos + offset + prefab.blocks[i].first, prefab.blocks[i].second.GetType());
                  }
                }
              }
            }
            if (wpos.y < (height - 1))
            {
                voxels.SetBlockType(wpos, BlockType::bDirt);
                if (Utils::noise(pos) < .01f)
                    voxels.SetBlockType(wpos, BlockType::bStone);
            }

            if (wpos.y >= height && wpos.y <= waterHeight)
              voxels.SetBlockType(wpos, BlockType::bWater);

            //if (density < -.02)
            //{
            //  voxels.SetBlockType(wpos, BlockType::bGrass);
            //}
            //if (density < -.03)
            //{
            //  voxels.SetBlockType(wpos, BlockType::bDirt);
            //}
            //if (density < -.04)
            //{
            //  voxels.SetBlockType(wpos, BlockType::bStone);
            //}

            //printf("%i\n", height);
          }
        }
      }

      //FastNoiseSIMD::FreeNoiseSet(noiseSet);
    }
    else
    {
      //printf("null chunk doe\n");
    }
  });

  std::vector<glm::ivec3> lightBlocks;

  // Place starting house after generating world
  Prefab prefab = PrefabManager::GetPrefab("placeholder_house");
  glm::ivec3 wpos = glm::ivec3{ 23, 2, 48 };
  for (unsigned i = 0; i < prefab.blocks.size(); i++)
  {
    if (auto block = voxels.TryGetBlock(wpos + prefab.blocks[i].first))
    {
      voxels.SetBlockType(wpos + prefab.blocks[i].first, prefab.blocks[i].second.GetType());
      if (prefab.blocks[i].second.GetEmittance() != glm::u8vec4{ 0,0,0,0 })
        lightBlocks.push_back(wpos + prefab.blocks[i].first);
    }
  }

  for (int i = 0; i < lightBlocks.size(); i++)
  {
    ChunkHelpers::localpos pos = ChunkHelpers::WorldPosToLocalPos(lightBlocks[i]);
    Block tmp = voxels.GetBlock(lightBlocks[i]);
    //voxels.SetBlock(lightBlocks[i], Block());
    //voxels.SetBlock(lightBlocks[i], { tmp.GetType(), tmp.GetEmittance() });
    voxels.UpdateBlock(lightBlocks[i], { tmp.GetType(), tmp.GetEmittance() });
    //Chunk* chunk = voxels.GetChunk(pos.chunk_pos);
    //voxels.chunkManager_->lightPropagateAdd(lightBlocks[i], .GetLightRef());
  }

  printf("Generating chunks took %f seconds\n", timer.Elapsed());
}


void WorldGen::InitMeshes()
{
  Timer timer;
  auto& chunks = voxels.chunks_;
  std::for_each(std::execution::par,
    chunks.begin(), chunks.end(), [](auto& p)
    {
      if (p)
      {
        p->BuildMesh();
      }
    });
  printf("Generating meshes took %f seconds\n", timer.Elapsed());
}


void WorldGen::InitBuffers()
{
  Timer timer;
  auto& chunks = voxels.chunks_;
  std::for_each(std::execution::seq,
    chunks.begin(), chunks.end(), [](auto& p)
  {
    if (p)
      p->BuildBuffers();
  });
  printf("Buffering meshes took %f seconds\n", timer.Elapsed());
}


bool WorldGen::checkDirectSunlight(glm::ivec3 wpos)
{
  auto p = ChunkHelpers::WorldPosToLocalPos(wpos);
  Voxels::Chunk* chunk = voxels.GetChunk(p.chunk_pos);
  if (!chunk)
    return false;
  Block block = chunk->BlockAt(p.block_pos);

  // find the highest valid chunk
  const glm::ivec3 up(0, 1, 0);
  glm::ivec3 cpos = p.chunk_pos + up;
  Voxels::Chunk* next = chunk;
  while (next)
  {
    chunk = next;
    cpos += up;
    next = voxels.GetChunk(cpos);
  }

  // go down until we hit another solid block or this block
  return false;
}

void WorldGen::InitializeSunlight()
{
  Timer timer;

  // find the max chunk height (assumed world has flat top, so no column-local max height needed)
  int maxY = std::numeric_limits<int>::min();
  int minY = std::numeric_limits<int>::max();

  for (const auto& chunk : voxels.chunks_)
  {
    if (chunk)
    {
      minY = glm::min(minY, chunk->GetPos().y);
      maxY = glm::max(maxY, chunk->GetPos().y);
    }
  }

  // generates initial columns of sunlight in the world
  for (auto chunk : voxels.chunks_)
  {
    // propagate light only from the highest chunks
    if (chunk == nullptr || chunk->GetPos().y != maxY)
      continue;
    auto cpos = chunk->GetPos();

    // for each block on top of the chunk
    for (int x = 0; x < Voxels::Chunk::CHUNK_SIZE; x++)
    {
      for (int z = 0; z < Voxels::Chunk::CHUNK_SIZE; z++)
      {
        glm::ivec3 lpos(x, Voxels::Chunk::CHUNK_SIZE - 1, z);
        Block curBlock = chunk->BlockAt(lpos);
        if (Block::PropertiesTable[curBlock.GetTypei()].visibility == Visibility::Opaque)
          continue;

        Light light = chunk->LightAtNoLock(lpos);
        light.SetS(0xF);
        chunk->SetLightAtNoLock(lpos, light);
        glm::ivec3 wpos = ChunkHelpers::LocalPosToWorldPos(lpos, cpos);
        lightsToPropagate.push(wpos);
      }
    }
  }

  while (!lightsToPropagate.empty())
  {
    glm::ivec3 wpos = lightsToPropagate.front();
    lightsToPropagate.pop();
    sunlightPropagateOnce(wpos);
  }

  printf("Sunlight propagation took %f seconds\n", timer.Elapsed());
}

// updates the sunlight of a block at a given location
void WorldGen::sunlightPropagateOnce(const glm::ivec3& wpos)
{
  // do something
  enum { left, right, up, down, front, back }; // +Z = front
  static constexpr glm::ivec3 dirs[] =
  {
    { 1, 0, 0 },
    {-1, 0, 0 },
    { 0, 1, 0 },
    { 0,-1, 0 },
    { 0, 0, 1 },
    { 0, 0,-1 },
  };

  auto wlpos = ChunkHelpers::WorldPosToLocalPos(wpos);
  Voxels::Chunk* cachedChunk = voxels.GetChunkNoCheck(wlpos.chunk_pos);
  bool dontUseCachedChunk = glm::any(glm::equal(wlpos.block_pos, glm::ivec3(0))) ||
    glm::any(glm::equal(wlpos.block_pos, glm::ivec3(Voxels::Chunk::CHUNK_SIZE - 1)));

  //Light curLight = voxels.GetBlock(wpos).GetLight();
  Light curLight = cachedChunk->LightAtNoLock(wlpos.block_pos);

  for (int dir = 0; dir < 6; dir++)
  {
    glm::ivec3 neighborPos = wpos + dirs[dir];
    auto lpos = ChunkHelpers::WorldPosToLocalPos(neighborPos);
    Voxels::Chunk* chunk;

    if (dontUseCachedChunk)
    {
      chunk = voxels.GetChunkNoCheck(lpos.chunk_pos);
    }
    else
    {
      chunk = cachedChunk;
    }

    if (!chunk)
      continue;

    Block neighbor = chunk->BlockAtNoLock(lpos.block_pos);

    if (Block::PropertiesTable[neighbor.GetTypei()].visibility == Visibility::Opaque)
      continue;

    Light neighborLight = neighbor.GetLight();

    if (neighborLight.GetS() + 2 > curLight.GetS())
      continue;

    if (curLight.GetS() == 0xF && dir == down)
    {
      neighborLight.SetS(0xF);
    }
    else
    {
      neighborLight.SetS(curLight.GetS() - 1);
    }
    //voxels.SetBlockLight(neighborPos, neighborLight);
    chunk->SetLightAtNoLock(lpos.block_pos, neighborLight);
    lightsToPropagate.push(neighborPos);
  }
}
