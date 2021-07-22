#pragma once
#include <voxel/block.h>
#include <shared_mutex>
#include <engine/gfx/StaticBuffer.h>
#include <engine/Physics.h>

namespace Voxels
{
  struct Chunk;
  class VoxelManager;

  class ChunkMesh
  {
  public:
    ChunkMesh(Chunk* p, const VoxelManager& vm) : parentChunk(p), voxelManager_(vm) {}
    ~ChunkMesh();

    void BuildBuffers();
    void BuildMesh();

    int64_t GetVertexCount() { return vertexCount_; }

    std::atomic_bool needsBuffering_ = false;

    enum AOStrength
    {
      AO_0,
      AO_1,
      AO_2,
      AO_3,

      AO_MIN = AO_0,
      AO_MAX = AO_3,
    };

  private:
    friend struct Chunk;

    void buildBlockFace(
      int face,
      const glm::ivec3& blockPos,
      BlockType block);
    void addQuad(const glm::ivec3& lpos, BlockType block, int face, const Chunk* nearChunk, Light light);
    int vertexFaceAO(const glm::vec3& lpos, const glm::vec3& cornerDir, const glm::vec3& norm);


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

    const VoxelManager& voxelManager_;
    const Chunk* parentChunk = nullptr;
    Chunk* parentCopy = nullptr;
    const Chunk* nearChunks[6]{};

    std::unique_ptr<GFX::StaticBuffer> encodedStuffVbo_;
    std::unique_ptr<GFX::StaticBuffer> lightingVbo_;
    std::unique_ptr<GFX::StaticBuffer> posVbo_;

    // vertex data (held until buffers are sent to GPU)
    std::vector<int> interleavedArr;
    std::vector<uint32_t> indices;
    uint32_t curIndex{};

    Physics::MeshCollider tCollider{};
    physx::PxRigidActor* tActor = nullptr;

    int64_t vertexCount_ = 0;
    uint64_t bufferHandle = 0;

    std::shared_mutex mtx;
  };
}