#pragma once
#include <Voxels/block.h>
#include <shared_mutex>
#include <CoreEngine/StaticBuffer.h>
#include <CoreEngine/Physics.h>

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

    GLsizei GetVertexCount() { return vertexCount_; }
    GLsizei GetPointCount() { return pointCount_; }

    // debug
    static inline bool debug_ignore_light_level = false;
    static inline std::atomic<double> accumtime = 0;
    static inline std::atomic<unsigned> accumcount = 0;

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
    std::vector<GLint> interleavedArr;

    Physics::MeshCollider tCollider{};
    physx::PxRigidActor* tActor = nullptr;

    GLsizei vertexCount_ = 0;
    uint64_t bufferHandle = NULL;

    GLsizei pointCount_ = 0;
    bool voxelReady_ = true; // hack to prevent same voxel from being added multiple times for splatting (I think)

    std::shared_mutex mtx;
  };
}