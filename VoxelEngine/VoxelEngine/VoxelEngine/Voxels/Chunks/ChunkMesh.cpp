#include <Chunks/ChunkMesh.h>
#include <Graphics/vao.h>
#include <Graphics/vbo.h>
#include <Graphics/dib.h>
#include <Chunks/Chunk.h>
#include <Chunks/ChunkHelpers.h>
#include <Chunks/ChunkStorage.h>
#include <Graphics/Vertices.h>
#include <Refactor/settings.h>
#include <Graphics/DynamicBuffer.h>
#include <Rendering/ChunkRenderer.h>

#include <iomanip>
#include <mutex>
#include <shared_mutex>
#include <chrono>
using namespace std::chrono;


ChunkMesh::~ChunkMesh()
{
  ChunkRenderer::allocator->Free(bufferHandle);
}

void ChunkMesh::Render()
{
  if (vao_)
  {
    if (pointCount_ == 0) return;
    vao_->Bind();
    dib_->Bind();
    //void* i[1] = { (void*)0 };
    //glMultiDrawElements(GL_TRIANGLES, &indexCount_, GL_UNSIGNED_INT, i, 1);
    //glDrawElementsInstanced(GL_TRIANGLES, indexCount_, GL_UNSIGNED_INT, (void*)0, 1);
    //glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (void*)0);
    glDrawArraysIndirect(GL_TRIANGLES, (void*)0);
    //glDrawArraysInstanced(GL_TRIANGLES, 0, vertexCount_, 1);
  }
}


void ChunkMesh::BuildBuffers2()
{
  std::lock_guard lk(mtx);

  namespace CR = ChunkRenderer;

  vertexCount_ = encodedStuffArr.size();

  CR::allocator->Free(bufferHandle);

  // nothing emitted, don't try to make buffers
  if (pointCount_ == 0)
    return;
  
  // free oldest allocations until there is enough space to allocate this buffer
  while ((bufferHandle = CR::allocator->Allocate(
    interleavedArr.data(), 
    interleavedArr.size() * sizeof(GLint), 
    this->parent->GetAABB())) == NULL)
  {
    CR::allocator->FreeOldest();
  }

  encodedStuffArr.clear();
  lightingArr.clear();
  interleavedArr.clear();

  //printf("%f: Buffered\n", glfwGetTime());
}


void ChunkMesh::BuildMesh()
{
  high_resolution_clock::time_point benchmark_clock_ = high_resolution_clock::now();

  for (int i = 0; i < fCount; i++)
  {
    nearChunks[i] = ChunkStorage::GetChunk(
      parent->GetPos() + ChunkHelpers::faces[i]);
  }

  {
    std::lock_guard lk(mtx);

    glm::ivec3 ap = parent->GetPos() * Chunk::CHUNK_SIZE;
    interleavedArr.push_back(ap.x);
    interleavedArr.push_back(ap.y);
    interleavedArr.push_back(ap.z);
    interleavedArr.push_back(12345); // necessary padding
    // no padding necessary

    glm::ivec3 pos;
    for (pos.z = 0; pos.z < Chunk::CHUNK_SIZE; pos.z++)
    {
      // precompute first flat index part
      int zcsq = pos.z * Chunk::CHUNK_SIZE_SQRED;
      for (pos.y = 0; pos.y < Chunk::CHUNK_SIZE; pos.y++)
      {
        // precompute second flat index part
        int yczcsq = pos.y * Chunk::CHUNK_SIZE + zcsq;
        for (pos.x = 0; pos.x < Chunk::CHUNK_SIZE; pos.x++)
        {
          // this is what we would be doing every innermost iteration
          //int index = x + y * CHUNK_SIZE + z * CHUNK_SIZE_SQRED;
          // we only need to do addition
          int index = pos.x + yczcsq;

          // skip fully transparent blocks
          BlockType block = parent->BlockTypeAt(index);
          if (Block::PropertiesTable[uint16_t(block)].visibility == Visibility::Invisible)
            continue;

          voxelReady_ = true;
          for (int f = Far; f < fCount; f++)
            buildBlockFace(f, pos, block);
          //buildBlockVertices_normal({ x, y, z }, block);
        }
      }
    }

    //printf("%f: Meshed\n", glfwGetTime());
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

  Chunk* nearChunk = parent;

  // if neighbor is out of this chunk, find which chunk it is in
  if (any(lessThan(nearblock.block_pos, ivec3(0))) || any(greaterThanEqual(nearblock.block_pos, ivec3(Chunk::CHUNK_SIZE))))
  {
    fastWorldPosToLocalPos(chunkPosToWorldPos(nearblock.block_pos, parent->GetPos()), nearblock);
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

//#pragma optimize("", off)
inline void ChunkMesh::addQuad(const glm::ivec3& lpos, BlockType block, int face, Chunk* nearChunk, Light light)
{
  if (voxelReady_)
  {
    pointCount_++;
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

    // compress attributes into 32 bits
    GLuint encoded = Encode(finalVert, normalIdx, texIdx, cindex);
    encodeds[cindex] = encoded;

    int invOcclusion = 6;
    if (Settings::Graphics.blockAO)
      invOcclusion = 2 * vertexFaceAO(lpos, vert, faces[face]);
    
    aoValues[cindex] = invOcclusion;
    invOcclusion = 6 - invOcclusion;
    auto tLight = light;
    tLight.Set(tLight.Get() - glm::min(tLight.Get(), glm::u8vec4(invOcclusion)));
    lighting = tLight.Raw();
    glm::ivec3 dirCent = glm::vec3(lpos) - glm::vec3(finalVert);
    //printf("(%d, %d, %d)\n", dirCent.x, dirCent.y, dirCent.z);
    lightdeds[cindex] = EncodeLight(lighting, dirCent);
  }

  const GLuint indicesA[6] = { 0, 1, 3, 3, 1, 2 }; // normal indices
  const GLuint indicesB[6] = { 0, 1, 2, 2, 3, 0 }; // anisotropy fix (flip tris)
  const GLuint* indices = indicesA;
  // partially solve anisotropy issue
  if (aoValues[0] + aoValues[2] > aoValues[1] + aoValues[3])
    indices = indicesB;
  for (int i = 0; i < 6; i++)
  {
    encodedStuffArr.push_back(encodeds[indices[i]]);
    lightingArr.push_back(lightdeds[indices[i]]);
    interleavedArr.push_back(encodeds[indices[i]]);
    interleavedArr.push_back(lightdeds[indices[i]]);
  }
}


// minimize amount of searching in the global chunk map
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
        if (parent->BlockAt(ivec3(sidePos)).GetType() != BlockType::bAir)
          occluded++;
    }
  }


  if (occluded == 2)
    return 0;

  vec3 cornerPos = lpos + (cornerDir * 2.0f);
  if (all(greaterThanEqual(cornerPos, vec3(0))) && all(lessThan(cornerPos, vec3(Chunk::CHUNK_SIZE))))
    if (parent->BlockAt(ivec3(cornerPos)).GetType() != BlockType::bAir)
      occluded++;

  return 3 - occluded;
}