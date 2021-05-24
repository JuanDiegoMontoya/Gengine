#include <Voxels/ChunkMesh.h>
#include <Voxels/Chunk.h>
#include <Voxels/ChunkHelpers.h>
#include <CoreEngine/Vertices.h>
#include <CoreEngine/DynamicBuffer.h>
#include <Voxels/ChunkRenderer.h>
#include <Voxels/VoxelManager.h>


#include <iomanip>
#include <mutex>
#include <shared_mutex>
#include <chrono>
using namespace std::chrono;

namespace Voxels
{
  ChunkMesh::~ChunkMesh()
  {
    voxelManager_.chunkRenderer_->allocator->Free(bufferHandle);
    if (tActor)
    {
      Physics::PhysicsManager::RemoveActorGeneric(tActor);
    }
  }


  void ChunkMesh::BuildBuffers()
  {
    std::lock_guard lk(mtx);

    // optimization in case this function is called multiple times in a row
    if (!needsBuffering_)
    {
      return;
    }
    needsBuffering_ = false;

    voxelManager_.chunkRenderer_->allocator->Free(bufferHandle);

    // nothing emitted, don't try to make buffers
    //if (pointCount_ == 0)
    if (vertexCount_ == 0)
    {
      return;
    }

    // free oldest allocations until there is enough space to allocate this buffer
    while ((bufferHandle = voxelManager_.chunkRenderer_->allocator->Allocate(
      interleavedArr.data(),
      interleavedArr.size() * sizeof(GLint),
      this->parentChunk->GetAABB())) == NULL)
    {
      voxelManager_.chunkRenderer_->allocator->FreeOldest();
    }

    interleavedArr.clear();

    tCollider.vertices.clear();
    tCollider.indices.clear();

    interleavedArr.shrink_to_fit();
    tCollider.vertices.shrink_to_fit();
    tCollider.indices.shrink_to_fit();
  }


  void ChunkMesh::BuildMesh()
  {
    high_resolution_clock::time_point benchmark_clock_ = high_resolution_clock::now();

    {
      std::lock_guard lk(mtx);
      needsBuffering_ = true;

      // clear everything in case this function is called twice in a row
      vertexCount_ = 0;
      interleavedArr.clear();
      tCollider.vertices.clear();
      tCollider.indices.clear();

      // make a copy of each of the parent and neighboring chunks so that they can be read without being updating while we mesh
      parentChunk->Lock();
      parentCopy = new Chunk(*parentChunk);
      parentChunk->Unlock();

      for (int i = 0; i < fCount; i++)
      {
        const Chunk* near = voxelManager_.GetChunk(parentCopy->GetPos() + ChunkHelpers::faces[i]);
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

        voxelReady_ = true;
        glm::vec3 pos
        {
          i % Chunk::CHUNK_SIZE,
          (i / Chunk::CHUNK_SIZE) % Chunk::CHUNK_SIZE,
          i / (Chunk::CHUNK_SIZE_SQRED)
        };
        for (int f = Far; f < fCount; f++)
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

    duration<double> benchmark_duration_ = duration_cast<duration<double>>(high_resolution_clock::now() - benchmark_clock_);
    double milliseconds = benchmark_duration_.count() * 1000;
    if (accumcount > 1000)
    {
      accumcount = 0;
      accumtime = 0;
    }
    accumtime = accumtime + milliseconds;
    accumcount = accumcount + 1;
    //std::cout 
    //  << std::setw(-2) << std::showpoint << std::setprecision(4) << accumtime / accumcount << " ms "
    //  << "(" << milliseconds << ")"
    //  << std::endl;
  }


  inline void ChunkMesh::buildBlockFace(
    int face,
    const glm::ivec3& blockPos,  // position of current block
    BlockType block)            // block-specific information)
  {
    using namespace glm;
    using namespace ChunkHelpers;
    thread_local static localpos nearblock; // avoids unnecessary construction of vec3s
    //glm::ivec3 nearFace = blockPos + faces[face];

    nearblock.block_pos = blockPos + faces[face];

    const Chunk* nearChunk = parentCopy;

    // if neighbor is out of this chunk, find which chunk it is in
    if (any(lessThan(nearblock.block_pos, ivec3(0))) || any(greaterThanEqual(nearblock.block_pos, ivec3(Chunk::CHUNK_SIZE))))
    {
      fastWorldPosToLocalPos(chunkPosToWorldPos(nearblock.block_pos, parentCopy->GetPos()), nearblock);
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

  inline void ChunkMesh::addQuad(const glm::ivec3& lpos, BlockType block, int face, const Chunk* nearChunk, Light light)
  {
    if (voxelReady_)
    {
      //pointCount_++;
      voxelReady_ = false;
    }

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
      GLuint encoded = EncodeVertex(finalVert, normalIdx, texIdx, cindex);
      encodeds[cindex] = encoded;

      int invOcclusion = 3;
      if (true) // TODO: make this an option in the future
      {
        invOcclusion = vertexFaceAO(lpos, vert, faces[face]);
      }

      aoValues[cindex] = invOcclusion;
      auto tLight = light;
      lighting = tLight.Raw();
      glm::ivec3 dirCent = glm::vec3(lpos) - glm::vec3(finalVert);
      lightdeds[cindex] = EncodeLight(lighting, dirCent, invOcclusion);
    }

    constexpr GLuint indicesA[6] = { 0, 1, 3, 3, 1, 2 }; // normal indices
    constexpr GLuint indicesB[6] = { 0, 1, 2, 2, 3, 0 }; // anisotropy fix (flip tris)
    const GLuint* indices = indicesA;
    // partially solve anisotropy issue
    if (aoValues[0] + aoValues[2] > aoValues[1] + aoValues[3])
    {
      indices = indicesB;
    }
    for (int i = 0; i < 6; i++)
    {
      vertexCount_++;
      interleavedArr.push_back(encodeds[indices[i]]);
      interleavedArr.push_back(lightdeds[indices[i]]);

      tCollider.indices.push_back(indicesA[i] + tCollider.vertices.size() - (tCollider.vertices.size() % 4) - 4);
    }
  }


  inline int ChunkMesh::vertexFaceAO(const glm::vec3& lpos, const glm::vec3& cornerDir, const glm::vec3& norm)
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

    return 3 - occluded;
  }
}