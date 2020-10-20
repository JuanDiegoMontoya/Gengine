#pragma once
#include <Voxels/block.h>
#include <shared_mutex>
#include <CoreEngine/StaticBuffer.h>

//class VAO;
//class VBO;
//class DIB;
struct Chunk;
class VoxelManager;

class ChunkMesh
{
public:
  ChunkMesh(Chunk* p, const VoxelManager& vm) : parent(p), voxelManager_(vm) {}
  ~ChunkMesh();

  void BuildBuffers();
  void BuildMesh();

  GLsizei GetVertexCount() { return vertexCount_; }
  GLsizei GetPointCount() { return pointCount_; }

  // debug
  static inline bool debug_ignore_light_level = false;
  static inline std::atomic<double> accumtime = 0;
  static inline std::atomic<unsigned> accumcount = 0;

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
  const Chunk* parent = nullptr;
  const Chunk* nearChunks[6]{ nullptr };

  std::unique_ptr<StaticBuffer> encodedStuffVbo_;
  std::unique_ptr<StaticBuffer> lightingVbo_;
  std::unique_ptr<StaticBuffer> posVbo_;

  // vertex data (held until buffers are sent to GPU)
  std::vector<GLint> encodedStuffArr;
  std::vector<GLint> lightingArr;
  std::vector<GLint> interleavedArr;

  GLsizei vertexCount_ = 0; // number of block vertices
  uint64_t bufferHandle = NULL;

  GLsizei pointCount_ = 0;
  bool voxelReady_ = true; // hack to prevent same voxel from being added multiple times for splatting (I think)

  std::shared_mutex mtx;
};


