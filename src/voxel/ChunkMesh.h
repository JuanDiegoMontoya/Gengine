#pragma once

namespace Voxels
{
  struct Chunk;
  class VoxelManager;
  namespace detail
  {
    struct ChunkMeshData;
  }

  class ChunkMesh
  {
  public:
    ChunkMesh(const Chunk* parent, const VoxelManager* vm);
    ~ChunkMesh();

    void BuildBuffers();
    void BuildMesh();

    const VoxelManager* GetVoxelManager() const;

    //int64_t GetVertexCount() { return vertexCount_; }

  private:
    detail::ChunkMeshData* data{};
  };
}