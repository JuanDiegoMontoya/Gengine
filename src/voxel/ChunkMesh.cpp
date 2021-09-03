#include "vPCH.h"
#include <voxel/ChunkMesh.h>
#include <voxel/Chunk.h>
#include <voxel/block.h>
#include <voxel/ChunkHelpers.h>
#include <voxel/ChunkRenderer.h>
#include <voxel/VoxelManager.h>

#include <engine/gfx/Vertices.h>
#include <engine/gfx/DynamicBuffer.h>
#include <engine/Physics.h>
#include <engine/CVar.h>

#include <glm/gtc/matrix_transform.hpp>

#include <iomanip>
#include <mutex>
#include <shared_mutex>
#include <bit>

#define DEBUG_ENCODING 1

namespace Voxels
{
  namespace detail
  {
    struct ChunkMeshData
    {
      const VoxelManager* voxelManager_;
      const Chunk* parentChunk = nullptr;
      Chunk* parentCopy = nullptr;
      const Chunk* nearChunks[6]{};
      std::atomic_bool needsBuffering_ = false;

      // vertex data (held until buffers are sent to GPU)
      std::vector<uint32_t> interleavedArr;
      uint32_t curIndex{};

      Physics::MeshCollider tCollider{};
      physx::PxRigidActor* tActor = nullptr;

      int64_t quadCount_ = 0;
      uint64_t bufferHandle = 0;

      std::shared_mutex mtx;

      void BuildBuffers();
      void BuildMesh();

      void buildBlockFace(
        int face,
        const glm::ivec3& blockPos,
        BlockType block);
      void addQuad(const glm::ivec3& lpos, BlockType block, int face, const Chunk* nearChunk, Light light);
      int vertexFaceAO(const glm::vec3& lpos, const glm::vec3& cornerDir, const glm::vec3& norm);
    };

    enum
    {
      Far,
      Near,
      Left,
      Right,
      Top,
      Bottom,

      fCount
    };

    enum AOStrength
    {
      AO_0,
      AO_1,
      AO_2,
      AO_3,

      AO_MIN = AO_0,
      AO_MAX = AO_3,
    };

    // counterclockwise from bottom right texture coordinates
    inline const glm::vec2 tex_corners[] =
    {
      { 1, 0 },
      { 1, 1 },
      { 0, 1 },
      { 0, 0 },
    };

    inline const glm::ivec3 faces[6] =
    {
      { 0, 0, 1 }, // 'far' face    (+z direction)
      { 0, 0,-1 }, // 'near' face   (-z direction)
      {-1, 0, 0 }, // 'left' face   (-x direction)
      { 1, 0, 0 }, // 'right' face  (+x direction)
      { 0, 1, 0 }, // 'top' face    (+y direction)
      { 0,-1, 0 }, // 'bottom' face (-y direction)
    };

    void DecodeQuad(uint32_t encoded, glm::uvec3& blockPos, uint32_t& face, uint32_t& texIdx)
    {
      // decode vertex position
      blockPos.x = encoded & 0x1F;        // = 0b11111
      blockPos.y = (encoded >> 5) & 0x1F;
      blockPos.z = (encoded >> 10) & 0x1F;

      // decode normal
      face = (encoded >> 15) & 0x7; // = 0b111

      ASSERT(face <= 5);

      // decode texture index and UV
      texIdx = (encoded >> 18) & 0x3FF; // = 0b1111111111 (10 bits)
      //uint32_t cornerIdx = (encoded >> 0) & 0x3; // = 0b11

      // NOTE: texCoord cannot be computed without VS built-in variables
      //texCoord = glm::vec3(tex_corners[cornerIdx], textureIdx);
    }


    uint32_t EncodeQuad(const glm::uvec3& blockPos, uint32_t normalIdx, uint32_t texIdx)
    {
      uint32_t encoded = 0;

      // encode vertex position
      encoded |= blockPos.x;
      encoded |= blockPos.y << 5;
      encoded |= blockPos.z << 10;

      // encode normal
      encoded |= normalIdx << 15;

      // encode texture information
      encoded |= texIdx << 18;

#if DEBUG_ENCODING
      ASSERT(std::countl_zero(texIdx) >= 22); // only least significant 10 bits should be used
      ASSERT(glm::all(glm::lessThanEqual(blockPos, glm::uvec3(31))));
      glm::uvec3 mdl{};
      uint32_t fce{};
      uint32_t txi{};
      DecodeQuad(encoded, mdl, fce, txi);
      ASSERT(mdl == blockPos);
      ASSERT(texIdx == txi);
      ASSERT(normalIdx == fce);
#endif

      return encoded;
    }


    // packs direction to center of block with lighting information
    uint32_t EncodeQuadLight(uint32_t lightEncoding, uint32_t ao)
    {
#if DEBUG_ENCODING
      ASSERT(std::countl_zero(lightEncoding) >= 16); // only the least significant 16 bits should be used
      ASSERT(std::countl_zero(ao) >= 24); // only the least significant 8 bits should be used
#endif

      uint32_t encoded = lightEncoding;

      encoded |= ao << 16;

      return encoded;
    }
  }



  void detail::ChunkMeshData::BuildBuffers()
  {
    std::lock_guard lk(mtx);

    // optimization in case this function is called multiple times in a row
    if (!needsBuffering_)
    {
      return;
    }
    needsBuffering_ = false;

    voxelManager_->chunkRenderer_->FreeChunkMesh(bufferHandle);
    bufferHandle = 0;

    // nothing emitted, don't try to make buffers
    if (quadCount_ == 0)
    {
      return;
    }

    bufferHandle = voxelManager_->chunkRenderer_->AllocChunkMesh(interleavedArr, parentChunk->GetAABB());

    interleavedArr.clear();
    tCollider.vertices.clear();
    tCollider.indices.clear();

    interleavedArr.shrink_to_fit();
    tCollider.vertices.shrink_to_fit();
    tCollider.indices.shrink_to_fit();
  }

  void detail::ChunkMeshData::BuildMesh()
  {
    std::lock_guard lk(mtx);
    needsBuffering_ = true;

    // clear everything in case this function is called twice in a row
    quadCount_ = 0;
    interleavedArr.clear();
    tCollider.vertices.clear();
    tCollider.indices.clear();

    // make a copy of each of the parent and neighboring chunks so that they can be read without being updating while we mesh
    parentChunk->Lock();
    parentCopy = new Chunk(*parentChunk);
    parentChunk->Unlock();

    for (int i = 0; i < fCount; i++)
    {
      const Chunk* near = voxelManager_->GetChunk(parentCopy->GetPos() + detail::faces[i]);
      if (near)
      {
        near->Lock();
        nearChunks[i] = new Chunk(*near);
        near->Unlock();
      }
    }

    glm::ivec3 ap = parentCopy->GetPos() * Chunk::CHUNK_SIZE;
    interleavedArr.push_back(ap.x);
    interleavedArr.push_back(ap.y);
    interleavedArr.push_back(ap.z);
    interleavedArr.push_back(0xDEADBEEF); // necessary padding

    for (size_t i = 0; i < Chunk::CHUNK_SIZE_CUBED; i++)
    {
      // skip fully transparent blocks
      BlockType block = parentCopy->BlockTypeAt(i);
      if (Block::PropertiesTable[uint16_t(block)].visibility == Visibility::Invisible)
      {
        continue;
      }

      glm::vec3 pos
      {
        i % Chunk::CHUNK_SIZE,
        (i / Chunk::CHUNK_SIZE) % Chunk::CHUNK_SIZE,
        i / (Chunk::CHUNK_SIZE_SQRED)
      };
      for (int f = 0; f < fCount; f++)
      {
        buildBlockFace(f, pos, block);
      }
    }


    Physics::PhysicsManager::RemoveActorGeneric(tActor);

    tActor = reinterpret_cast<physx::PxRigidActor*>(
      Physics::PhysicsManager::AddStaticActorGeneric(
        Physics::MaterialType::TERRAIN, tCollider,
        glm::translate(glm::mat4(1), glm::vec3(parentCopy->GetPos() * Chunk::CHUNK_SIZE))));

    for (int i = 0; i < fCount; i++)
    {
      delete nearChunks[i];
      nearChunks[i] = nullptr;
    }
    delete parentCopy;
    parentCopy = nullptr;
  }


  void detail::ChunkMeshData::buildBlockFace(
    int face,
    const glm::ivec3& blockPos,  // position of current block
    BlockType block)            // block-specific information)
  {
    using namespace glm;
    using namespace ChunkHelpers;
    thread_local static localpos nearblock; // avoids unnecessary construction of vec3s
    //glm::ivec3 nearFace = blockPos + faces[face];

    nearblock.block_pos = blockPos + detail::faces[face];

    const Chunk* nearChunk = parentCopy;

    // if neighbor is out of this chunk, find which chunk it is in
    if (any(lessThan(nearblock.block_pos, ivec3(0))) || any(greaterThanEqual(nearblock.block_pos, ivec3(Chunk::CHUNK_SIZE))))
    {
      WorldPosToLocalPosFast(LocalPosToWorldPos(nearblock.block_pos, parentCopy->GetPos()), nearblock);
      nearChunk = nearChunks[face];
    }

    // for now, we won't make a mesh for faces adjacent to NULL chunks
    // in the future it may be wise to construct the mesh regardless
    if (nearChunk == nullptr)
    {
      addQuad(blockPos, block, face, nearChunk, Light({ 0, 0, 0, 15 }));
      return;
    }

    // neighboring block and light
    Block block2 = nearChunk->BlockAt(nearblock.block_pos);
    Light light = block2.GetLight();
    //Light light = nearChunk->LightAtCheap(nearblock.block_pos);

    // this block is water and other block isn't water and is above this block
    if ((block2.GetType() != BlockType::bWater && block == BlockType::bWater && (nearblock.block_pos - blockPos).y > 0) ||
      Block::PropertiesTable[block2.GetTypei()].visibility > Visibility::Opaque)
    {
      addQuad(blockPos, block, face, nearChunk, light);
      return;
    }
    // other block isn't air or water - don't add mesh
    if (block2.GetType() != BlockType::bAir && block2.GetType() != BlockType::bWater)
      return;
    // both blocks are water - don't add mesh
    if (block2.GetType() == BlockType::bWater && block == BlockType::bWater)
      return;
    // this block is invisible - don't add mesh
    if (Block::PropertiesTable[uint16_t(block)].visibility == Visibility::Invisible)
      return;

    // if all tests are passed, generate this face of the block
    addQuad(blockPos, block, face, nearChunk, light);
  }

  void detail::ChunkMeshData::addQuad(const glm::ivec3& lpos, BlockType block, int face, [[maybe_unused]] const Chunk* nearChunk, Light light)
  {
    quadCount_++;
    uint32_t normalIdx = face;
    uint32_t texIdx = static_cast<uint32_t>(block);

    uint32_t aoValues = 0; // pack all 4 vertices' AO values into a u32 (2 bits each)
    const float* data = Vertices::cube_light;
    uint32_t endQuad = (face + 1) * 12;
    for (uint32_t i = face * 12, vertexIndex = 0; i < endQuad; i += 3, vertexIndex++) // cindex = corner index
    {
      // transform vertices relative to chunk
      glm::vec3 vert(data[i + 0], data[i + 1], data[i + 2]);
      glm::uvec3 finalVert = glm::ceil(vert) + glm::vec3(lpos);// +0.5f;

      tCollider.vertices.push_back(glm::vec3(finalVert));

      uint32_t vertexAO = AO_MIN;
      if (true) // TODO: make this an option in the future
      {
        vertexAO = vertexFaceAO(lpos, vert, detail::faces[face]);
      }
      aoValues |= vertexAO << (2 * vertexIndex);
    }

    constexpr uint32_t indicesA[6] = { 0, 1, 3, 3, 1, 2 }; // normal indices
    //constexpr uint32_t indicesB[6] = { 0, 1, 2, 2, 3, 0 }; // anisotropy fix (flip tris)
    //const uint32_t* indicesC = indicesA;
    // partially solve anisotropy issue
    // UPDATED: no need, as we now do bilinear interpolation
    //if (aoValues[0] + aoValues[2] > aoValues[1] + aoValues[3])
    //{
    //  indicesC = indicesB;
    //}
    
    // compress attributes into 32 bits
    interleavedArr.push_back(detail::EncodeQuad(lpos, normalIdx, texIdx));
    interleavedArr.push_back(detail::EncodeQuadLight(light.raw, aoValues));
    for (int i = 0; i < 6; i++)
    {
      //tCollider.indices.push_back(curIndex + indicesA[i]);
      //indices.push_back(curIndex + indicesA[i]);
      tCollider.indices.push_back(indicesA[i] + tCollider.vertices.size() - (tCollider.vertices.size() % 4) - 4);
    }
  }


  int detail::ChunkMeshData::vertexFaceAO(const glm::vec3& lpos, const glm::vec3& cornerDir, const glm::vec3& norm)
  {
    // TODO: make it work over chunk boundaries
    using namespace glm;

    int occluded = 0;

    // sides are systems of the corner minus the normal direction
    vec3 sidesDir = cornerDir * 2.0f - norm;
    for (int i = 0; i < sidesDir.length(); i++)
    {
      if (sidesDir[i] != 0)
      {
        vec3 sideDir(0);
        sideDir[i] = sidesDir[i];
        vec3 sidePos = lpos + sideDir + norm;
        if (all(greaterThanEqual(sidePos, vec3(0))) && all(lessThan(sidePos, vec3(Chunk::CHUNK_SIZE))))
          if (parentCopy->BlockAtNoLock(ivec3(sidePos)).GetType() != BlockType::bAir)
            occluded++;
      }
    }


    if (occluded == 2)
      return 0;

    vec3 cornerPos = lpos + (cornerDir * 2.0f);
    if (all(greaterThanEqual(cornerPos, vec3(0))) && all(lessThan(cornerPos, vec3(Chunk::CHUNK_SIZE))))
      if (parentCopy->BlockAtNoLock(ivec3(cornerPos)).GetType() != BlockType::bAir)
        occluded++;

    return AO_MAX - occluded;
  }

  ChunkMesh::ChunkMesh(const Chunk* parent, const VoxelManager* vm)
  {
    data = new detail::ChunkMeshData;
    data->parentChunk = parent;
    data->voxelManager_ = vm;
  }

  ChunkMesh::~ChunkMesh()
  {
    data->voxelManager_->chunkRenderer_->FreeChunkMesh(data->bufferHandle);
    if (data->tActor)
    {
      Physics::PhysicsManager::RemoveActorGeneric(data->tActor);
    }
    delete data;
  }

  void ChunkMesh::BuildBuffers()
  {
    data->BuildBuffers();
  }


  void ChunkMesh::BuildMesh()
  {
    data->BuildMesh();
  }

  const VoxelManager* ChunkMesh::GetVoxelManager() const
  {
    return data->voxelManager_;
  }
}
