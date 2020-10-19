#pragma once
#include <block.h>
#include <light.h>
#include <mutex>
#include <Graphics/Shapes.h>
#include <concurrent_vector.h>
#include <Chunks/ChunkHelpers.h>
#include <Chunks/BlockStorage.h>
#include <Chunks/ChunkMesh.h>
#include <GAssert.h>
#include <cereal/archives/binary.hpp>

// TODO: make these constexpr functions!
#define ID3D(x, y, z, h, w) (x + h * (y + w * z))
#define ID2D(x, y, w) (w * y + x)


//typedef std::pair<glm::ivec3, glm::ivec3> localpos;

/*
  0: -x-y+z
  1: +x-y+z
  2: +x-y-z
  3: -x-y-z
  4: -x+y+z
  5: +x+y+z
  6: +x+y-z
  7: -x+y-z
*/

class VoxelManager;

struct Chunk
{
private:
public:
  Chunk(const VoxelManager& vm) : pos_(0), mesh(this, vm) { ASSERT(0); }
  Chunk(const glm::ivec3& p, const VoxelManager& vm) : pos_(p), mesh(this, vm) {}
  ~Chunk() {};
  Chunk(const Chunk& other);
  Chunk& operator=(const Chunk& rhs);

  /*################################
            Global Chunk Info
  ################################*/
  static constexpr int CHUNK_SIZE        = 32;
  static constexpr int CHUNK_SIZE_SQRED = CHUNK_SIZE * CHUNK_SIZE;
  static constexpr int CHUNK_SIZE_CUBED = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;
  static constexpr int CHUNK_SIZE_LOG2  = 5; // log2(32) = 5


  /*################################
          Query Functions
  ################################*/
  inline const glm::ivec3& GetPos() const { return pos_; }

  inline Block BlockAt(const glm::ivec3& p) const
  {
    return storage.GetBlock(ID3D(p.x, p.y, p.z, CHUNK_SIZE, CHUNK_SIZE));
  }

  inline Block BlockAt(int index) const
  {
    return storage.GetBlock(index);
  }

  inline BlockType BlockTypeAt(const glm::ivec3& p) const
  {
    return storage.GetBlockType(ID3D(p.x, p.y, p.z, CHUNK_SIZE, CHUNK_SIZE));
  }

  inline BlockType BlockTypeAt(int index) const
  {
    return storage.GetBlockType(index);
  }

  inline void SetBlockTypeAt(const glm::ivec3& lpos, BlockType type)
  {
    int index = ID3D(lpos.x, lpos.y, lpos.z, CHUNK_SIZE, CHUNK_SIZE);
    storage.SetBlock(index, type);
  }

  inline void SetLightAt(const glm::ivec3& lpos, Light light)
  {
    int index = ID3D(lpos.x, lpos.y, lpos.z, CHUNK_SIZE, CHUNK_SIZE);
    storage.SetLight(index, light);
  }

  inline Light LightAt(const glm::ivec3& p) const
  {
    return storage.GetLight(ID3D(p.x, p.y, p.z, CHUNK_SIZE, CHUNK_SIZE));
  }

  inline Light LightAt(int index) const
  {
    return storage.GetLight(index);
  }


  inline void SetBlockTypeAtIndirect(const glm::ivec3& lpos, BlockType type)
  {
    std::lock_guard lk(indirectMtx_);
    int index = ID3D(lpos.x, lpos.y, lpos.z, CHUNK_SIZE, CHUNK_SIZE);
    auto light = LightAt(index);
    updateQueue_.push_back({ index, Block(type, light) });
  }

  inline void SetLightAtIndirect(const glm::ivec3& lpos, Light light)
  {
    std::lock_guard lk(indirectMtx_);
    int index = ID3D(lpos.x, lpos.y, lpos.z, CHUNK_SIZE, CHUNK_SIZE);
    auto block = BlockTypeAt(index);
    updateQueue_.push_back({ index, Block(block, light) });
  }

  void BuildMesh()
  {
    mesh.BuildMesh();
  }

  void BuildBuffers()
  {
    //mesh.BuildBuffers();
    mesh.BuildBuffers();
  }

  ChunkMesh& GetMesh()
  {
    return mesh;
  }

  AABB GetAABB() const
  {
    return {
      glm::vec3(pos_ * CHUNK_SIZE),
      glm::vec3(pos_ * CHUNK_SIZE + CHUNK_SIZE) };
  }

  // Serialization
  template <class Archive>
  void serialize(Archive& ar)
  {
    ar(pos_, storage);
  }

  const PaletteBlockStorage<CHUNK_SIZE_CUBED>& GetStorage() const { return storage; }

private:
  glm::ivec3 pos_;  // position relative to other chunks (1 chunk = 1 index)

  //ArrayBlockStorage<CHUNK_SIZE_CUBED> storage;
  PaletteBlockStorage<CHUNK_SIZE_CUBED> storage;
  ChunkMesh mesh;

  // for deferred block updates
  std::vector<std::pair<int, Block>> updateQueue_;
  //concurrency::concurrent_vector<std::pair<int, Block>> updateQueue_;
  std::shared_mutex indirectMtx_;
};
