#pragma once
#include <Voxels/block.h>
#include <Voxels/light.h>
#include <mutex>
#include <CoreEngine/Shapes.h>
#include <concurrent_vector.h>
#include <Voxels/ChunkHelpers.h>
#include <Voxels/BlockStorage.h>
#include <Voxels/ChunkMesh.h>
#include <CoreEngine/GAssert.h>
#include <cereal/archives/binary.hpp>

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
  static constexpr int CHUNK_SIZE       = 32;
  static constexpr int CHUNK_SIZE_SQRED = CHUNK_SIZE * CHUNK_SIZE;
  static constexpr int CHUNK_SIZE_CUBED = CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;
  static constexpr int CHUNK_SIZE_LOG2  = 5;
  static constexpr int BLOCKS_PER_X = 1;
  static constexpr int BLOCKS_PER_Y = CHUNK_SIZE;
  static constexpr int BLOCKS_PER_Z = CHUNK_SIZE_SQRED;
  static inline const glm::ivec3 BLOCKS_PER_DIM{ BLOCKS_PER_X, BLOCKS_PER_Y, BLOCKS_PER_Z };

  inline const glm::ivec3& GetPos() const { return pos_; }

  Block BlockAt(const glm::ivec3& p) const;
  Block BlockAt(int index) const;
  Block BlockAtNoLock(const glm::ivec3& p) const;
  Block BlockAtNoLock(int index) const;
  BlockType BlockTypeAt(const glm::ivec3& p) const;
  BlockType BlockTypeAt(int index) const;
  BlockType BlockTypeAtNoLock(const glm::ivec3& p) const;
  BlockType BlockTypeAtNoLock(int index) const;
  void SetBlockTypeAt(const glm::ivec3& lpos, BlockType type);
  void SetBlockTypeAtNoLock(const glm::ivec3& localPos, BlockType type);
  void SetLightAt(const glm::ivec3& lpos, Light light);
  void SetLightAtNoLock(const glm::ivec3& localPos, Light light);
  Light LightAt(const glm::ivec3& p) const;
  Light LightAt(int index) const;
  Light LightAtNoLock(const glm::ivec3& p) const;
  Light LightAtNoLock(int index) const;


  inline void Lock() const
  {
    mutex_.lock();
  }

  inline void Unlock() const
  {
    mutex_.unlock();
  }

  void BuildMesh()
  {
    mesh.BuildMesh();
  }

  void BuildBuffers()
  {
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

  const auto& GetStorage() const { return storage; }

private:
  glm::ivec3 pos_;  // position relative to other chunks (1 chunk = 1 index)

  ArrayBlockStorage<CHUNK_SIZE_CUBED> storage;
  //PaletteBlockStorage<CHUNK_SIZE_CUBED> storage;
  ChunkMesh mesh;

  mutable std::shared_mutex mutex_;
};



inline Block Chunk::BlockAt(const glm::ivec3& p) const
{
  int index = ChunkHelpers::IndexFrom3D(p.x, p.y, p.z, CHUNK_SIZE, CHUNK_SIZE);
  std::shared_lock lck(mutex_);
  return storage.GetBlock(index);
}

inline Block Chunk::BlockAt(int index) const
{
  std::shared_lock lck(mutex_);
  return storage.GetBlock(index);
}

inline Block Chunk::BlockAtNoLock(const glm::ivec3& p) const
{
  int index = ChunkHelpers::IndexFrom3D(p.x, p.y, p.z, CHUNK_SIZE, CHUNK_SIZE);
  return storage.GetBlock(index);
}

inline Block Chunk::BlockAtNoLock(int index) const
{
  return storage.GetBlock(index);
}

inline BlockType Chunk::BlockTypeAt(const glm::ivec3& p) const
{
  int index = ChunkHelpers::IndexFrom3D(p.x, p.y, p.z, CHUNK_SIZE, CHUNK_SIZE);
  std::shared_lock lck(mutex_);
  return storage.GetBlockType(index);
}

inline BlockType Chunk::BlockTypeAt(int index) const
{
  std::shared_lock lck(mutex_);
  return storage.GetBlockType(index);
}

inline BlockType Chunk::BlockTypeAtNoLock(const glm::ivec3& p) const
{
  int index = ChunkHelpers::IndexFrom3D(p.x, p.y, p.z, CHUNK_SIZE, CHUNK_SIZE);
  return storage.GetBlockType(index);
}

inline BlockType Chunk::BlockTypeAtNoLock(int index) const
{
  return storage.GetBlockType(index);
}

inline void Chunk::SetBlockTypeAt(const glm::ivec3& lpos, BlockType type)
{
  int index = ChunkHelpers::IndexFrom3D(lpos.x, lpos.y, lpos.z, CHUNK_SIZE, CHUNK_SIZE);
  std::lock_guard lck(mutex_);
  storage.SetBlock(index, type);
}

inline void Chunk::SetLightAt(const glm::ivec3& lpos, Light light)
{
  int index = ChunkHelpers::IndexFrom3D(lpos.x, lpos.y, lpos.z, CHUNK_SIZE, CHUNK_SIZE);
  std::lock_guard lck(mutex_);
  storage.SetLight(index, light);
}

inline Light Chunk::LightAt(const glm::ivec3& p) const
{
  int index = ChunkHelpers::IndexFrom3D(p.x, p.y, p.z, CHUNK_SIZE, CHUNK_SIZE);
  std::shared_lock lck(mutex_);
  return storage.GetLight(index);
}

inline Light Chunk::LightAt(int index) const
{
  std::shared_lock lck(mutex_);
  return storage.GetLight(index);
}

inline Light Chunk::LightAtNoLock(const glm::ivec3& p) const
{
  int index = ChunkHelpers::IndexFrom3D(p.x, p.y, p.z, CHUNK_SIZE, CHUNK_SIZE);
  return storage.GetLight(index);
}

inline Light Chunk::LightAtNoLock(int index) const
{
  return storage.GetLight(index);
}

inline void Chunk::SetBlockTypeAtNoLock(const glm::ivec3& localPos, BlockType type)
{
  int index = ChunkHelpers::IndexFrom3D(localPos.x, localPos.y, localPos.z, CHUNK_SIZE, CHUNK_SIZE);
  storage.SetBlock(index, type);
}

inline void Chunk::SetLightAtNoLock(const glm::ivec3& localPos, Light light)
{
  int index = ChunkHelpers::IndexFrom3D(localPos.x, localPos.y, localPos.z, CHUNK_SIZE, CHUNK_SIZE);
  storage.SetLight(index, light);
}
