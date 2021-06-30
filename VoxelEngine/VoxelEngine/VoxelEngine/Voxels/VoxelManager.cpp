#include "vPCH.h"
#include <Voxels/VoxelManager.h>
#include <Voxels/ChunkManager.h>
#include <Voxels/ChunkRenderer.h>
#include <Voxels/EditorRefactor.h>

namespace Voxels
{
  VoxelManager::VoxelManager(Scene& scene) : scene_(scene)
  {
    chunkManager_ = std::make_unique<ChunkManager>(*this);
    chunkManager_->Init();
    chunkRenderer_ = std::make_unique<ChunkRenderer>();
    editor_ = std::make_unique<Editor>(*this);
  }

  VoxelManager::~VoxelManager()
  {
    chunkManager_->Destroy();
  }

  void VoxelManager::Update()
  {
    chunkManager_->Update();
    chunkRenderer_->Update();
  }

  void VoxelManager::Draw()
  {
    chunkRenderer_->Draw();
    chunkRenderer_->DrawBuffers();
    editor_->Update();
  }

  void VoxelManager::UpdateBlock(const glm::ivec3& wpos, Block block)
  {
    chunkManager_->UpdateBlock(wpos, block);
  }

  void VoxelManager::UpdateBlockCheap(const glm::ivec3& wpos, Block block)
  {
    chunkManager_->UpdateBlockCheap(wpos, block);
  }

  void VoxelManager::UpdateChunk(const glm::ivec3& cpos)
  {
    //auto it = chunks_.find(cpos);
    //ASSERT(it != chunks_.end());
    //chunkManager_->UpdateChunk(it->second);
    chunkManager_->UpdateChunk(chunks_[flatten(cpos)]);
  }

  void VoxelManager::UpdateChunk(Chunk* chunk)
  {
    chunkManager_->UpdateChunk(chunk);
  }



  float mod(float value, float modulus)
  {
    return fmod((fmod(value, modulus)) + modulus, modulus);
    //return (value % modulus + modulus) % modulus;
  }

  float intbound(float s, float ds)
  {
    // Find the smallest positive t such that s+t*ds is an integer.
    if (ds < 0)
    {
      return intbound(-s, -ds);
    }
    else
    {
      s = mod(s, 1);
      // problem is now s+t*ds = 1
      return (1 - s) / ds;
    }
  }

  glm::vec3 intbound(glm::vec3 s, glm::vec3 ds)
  {
    return { intbound(s.x, ds.x), intbound(s.y, ds.y), intbound(s.z, ds.z) };
  }

  int signum(float x)
  {
    return x > 0 ? 1 : x < 0 ? -1 : 0;
  }

  /**
   * Call the callback with (x,y,z,value,face) of all blocks along the line
   * segment from point 'origin' in vector direction 'direction' of length
   * 'distance'. 'distance' may be infinite.
   *
   * 'face' is the normal vector of the face of that block that was entered.
   * It should not be used after the callback returns.
   *
   * If the callback returns a true value, the traversal will be stopped.
   */
  void VoxelManager::Raycast(glm::vec3 origin, glm::vec3 direction, float distance, std::function<bool(glm::vec3, Block, glm::vec3)> callback)
  {
    // From "A Fast Voxel Traversal Algorithm for Ray Tracing"
    // by John Amanatides and Andrew Woo, 1987
    // <http://www.cse.yorku.ca/~amana/research/grid.pdf>
    // <http://citeseer.ist.psu.edu/viewdoc/summary?doi=10.1.1.42.3443>
    // Extensions to the described algorithm:
    //   • Imposed a distance limit.
    //   • The face passed through to reach the current cube is provided to
    //     the callback.

    // The foundation of this algorithm is a parameterized representation of
    // the provided ray,
    //                    origin + t * direction,
    // except that t is not actually stored; rather, at any given point in the
    // traversal, we keep track of the *greater* t values which we would have
    // if we took a step sufficient to cross a cube boundary along that axis
    // (i.e. change the integer part of the coordinate) in the variables
    // tMax.x, tMax.y, and tMax.z.

    // Cube containing origin point.
    glm::ivec3 p = glm::floor(origin);
    // Break out direction vector.
    glm::vec3 d = direction;
    // Direction to increment x,y,z when stepping.
    glm::ivec3 step = glm::sign(d);
    // See description above. The initial values depend on the fractional
    // part of the origin.
    glm::vec3 tMax = intbound(origin, d);
    // The change in t when taking a step (always positive).
    glm::vec3 tDelta = glm::vec3(step) / d;
    // Buffer for reporting faces to the callback.
    glm::vec3 face(0); // probably needs to point in the direction it faces

    // Avoids an infinite loop.
    ASSERT_MSG(d != glm::vec3(0), "Raycast in zero direction!");

    // Rescale from units of 1 cube-edge to units of 'direction' so we can
    // compare with 't'.
    distance /= glm::length(d);

    while (1)
    {

      // Invoke the callback, unless we are not *yet* within the bounds of the
      // world.
      auto block = TryGetBlock(p);
      if (callback(p, block ? *block : Block{}, face))
        break;

      // tMax.x stores the t-value at which we cross a cube boundary along the
      // X axis, and similarly for Y and Z. Therefore, choosing the least tMax
      // chooses the closest cube boundary. Only the first case of the four
      // has been commented in detail.
      if (tMax.x < tMax.y)
      {
        if (tMax.x < tMax.z)
        {
          if (tMax.x > distance)
            break;
          // Update which cube we are now in.
          p.x += step.x;
          // Adjust tMax.x to the next X-oriented boundary crossing.
          tMax.x += tDelta.x;
          // Record the normal vector of the cube face we entered.
          face.x = float(-step.x);
          face.y = 0;
          face.z = 0;
        }
        else
        {
          if (tMax.z > distance)
            break;
          p.z += step.z;
          tMax.z += tDelta.z;
          face.x = 0;
          face.y = 0;
          face.z = float(-step.z);
        }
      }
      else
      {
        if (tMax.y < tMax.z)
        {
          if (tMax.y > distance)
            break;
          p.y += step.y;
          tMax.y += tDelta.y;
          face.x = 0;
          face.y = float(-step.y);
          face.z = 0;
        }
        else
        {
          // Identical to the second case, repeated for simplicity in
          // the conditionals.
          if (tMax.z > distance)
            break;
          p.z += step.z;
          tMax.z += tDelta.z;
          face.x = 0;
          face.y = 0;
          face.z = float(-step.z);
        }
      }
    }
  }
}