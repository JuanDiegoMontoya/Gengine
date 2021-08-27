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
      std::vector<int> interleavedArr;
      std::vector<uint32_t> indices;
      uint32_t curIndex{};

      Physics::MeshCollider tCollider{};
      physx::PxRigidActor* tActor = nullptr;

      int64_t vertexCount_ = 0;
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

    inline void Decode(GLuint encoded, glm::uvec3& modelPos, glm::vec3& normal, glm::vec3& texCoord)
    {
      // decode vertex position
      modelPos.x = encoded >> 26;
      modelPos.y = (encoded >> 20) & 0x3F; // = 0b111111
      modelPos.z = (encoded >> 14) & 0x3F; // = 0b111111

      // decode normal
      GLuint normalIdx = (encoded >> 11) & 0x7; // = 0b111
      ASSERT(normalIdx <= 5);
      if (normalIdx <= 5)
      {
        normal = faces[normalIdx];
      }

      // decode texture index and UV
      GLuint textureIdx = (encoded >> 2) & 0x1FF; // = 0b1111111111
      GLuint cornerIdx = (encoded >> 0) & 0x3; // = 0b11

      texCoord = glm::vec3(tex_corners[cornerIdx], textureIdx);
    }


    inline GLuint EncodeVertex(const glm::uvec3& modelPos, GLuint normalIdx, GLuint texIdx, GLuint cornerIdx)
    {
      GLuint encoded = 0;

      // encode vertex position
      encoded |= modelPos.x << 26;
      encoded |= modelPos.y << 20;
      encoded |= modelPos.z << 14;

      // encode normal
      encoded |= normalIdx << 11;

      // encode texture information
      encoded |= texIdx << 2;
      encoded |= cornerIdx << 0;

#if 0 // debug
      glm::uvec3 mdl;
      glm::vec3 nml, txc;
      Decode(encoded, mdl, nml, txc);
      ASSERT(mdl == modelPos);
      ASSERT(texIdx == txc.z);
      ASSERT(faces[normalIdx] == nml)
#endif

        return encoded;
    }


    // packs direction to center of block with lighting information
    inline uint32_t EncodeLight(uint32_t lightCoding, glm::ivec3 dirCent, uint32_t ao)
    {
      uint32_t encoded = lightCoding;
      dirCent = (dirCent + 1);
      //printf("(%d, %d, %d)\n", dirCent.x, dirCent.y, dirCent.z);

      using namespace glm;
      ASSERT(all(greaterThanEqual(dirCent, ivec3(0, 0, 0))) &&
        all(lessThanEqual(dirCent, ivec3(1, 1, 1))) &&
        ao <= detail::AO_MAX);

      encoded |= ao << 19;

      encoded |= dirCent.x << 18;
      encoded |= dirCent.y << 17;
      encoded |= dirCent.z << 16;

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

    voxelManager_->chunkRenderer_->FreeChunk(bufferHandle);
    bufferHandle = 0;

    // nothing emitted, don't try to make buffers
    if (vertexCount_ == 0)
    {
      return;
    }

    bufferHandle = voxelManager_->chunkRenderer_->AllocChunk(interleavedArr, indices, parentChunk->GetAABB());

    interleavedArr.clear();
    tCollider.vertices.clear();
    tCollider.indices.clear();
    indices.clear();

    interleavedArr.shrink_to_fit();
    tCollider.vertices.shrink_to_fit();
    tCollider.indices.shrink_to_fit();
    indices.shrink_to_fit();
    curIndex = 0;
  }

  void detail::ChunkMeshData::BuildMesh()
  {
    std::lock_guard lk(mtx);
    needsBuffering_ = true;

    // clear everything in case this function is called twice in a row
    vertexCount_ = 0;
    curIndex = 0;
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
    int normalIdx = face;
    int texIdx = (int)block; // temp value
    uint16_t lighting = light.Raw();
    //light.SetS(15); // max sunlight for testing

    // add 4 vertices representing a quad
    float aoValues[4] = { 0, 0, 0, 0 }; // AO for each quad
    GLuint encodeds[4] = { 0 };
    GLuint lightdeds[4] = { 0 };
    //int aoValuesIndex = 0;
    const GLfloat* data = Vertices::cube_light;
    int endQuad = (face + 1) * 12;
    for (int i = face * 12, cindex = 0; i < endQuad; i += 3, cindex++) // cindex = corner index
    {
      using namespace ChunkHelpers;
      // transform vertices relative to chunk
      glm::vec3 vert(data[i + 0], data[i + 1], data[i + 2]);
      glm::uvec3 finalVert = glm::ceil(vert) + glm::vec3(lpos);// +0.5f;

      tCollider.vertices.push_back(glm::vec3(finalVert));

      // compress attributes into 32 bits
      GLuint encoded = detail::EncodeVertex(finalVert, normalIdx, texIdx, cindex);
      encodeds[cindex] = encoded;

      int invOcclusion = AO_MIN;
      if (true) // TODO: make this an option in the future
      {
        invOcclusion = vertexFaceAO(lpos, vert, detail::faces[face]);
      }

      aoValues[cindex] = invOcclusion;
      lighting = light.Raw();
      glm::ivec3 dirCent = glm::vec3(lpos) - glm::vec3(finalVert);
      lightdeds[cindex] = detail::EncodeLight(lighting, dirCent, invOcclusion);
    }

    constexpr uint32_t indicesA[6] = { 0, 1, 3, 3, 1, 2 }; // normal indices
    constexpr uint32_t indicesB[6] = { 0, 1, 2, 2, 3, 0 }; // anisotropy fix (flip tris)
    const uint32_t* indicesC = indicesA;
    // partially solve anisotropy issue
    if (aoValues[0] + aoValues[2] > aoValues[1] + aoValues[3])
    {
      indicesC = indicesB;
    }
    for (int i = 0; i < 4; i++)
    {
      vertexCount_++;
      interleavedArr.push_back(encodeds[i]);
      interleavedArr.push_back(lightdeds[i]);
    }
    for (int i = 0; i < 6; i++)
    {
      tCollider.indices.push_back(curIndex + indicesA[i]);
      indices.push_back(curIndex + indicesA[i]);
      //tCollider.indices.push_back(indicesA[i] + tCollider.vertices.size() - (tCollider.vertices.size() % 4) - 4);
    }
    curIndex += 4;
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
    data->voxelManager_->chunkRenderer_->FreeChunk(data->bufferHandle);
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
