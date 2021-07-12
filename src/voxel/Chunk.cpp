#include "vPCH.h"
#include <voxel/Chunk.h>

namespace Voxels
{
  Chunk::Chunk(const Chunk& other) : mesh(this, other.mesh.voxelManager_)
  {
    *this = other;
  }

  // copy assignment operator for serialization
  Chunk& Chunk::operator=(const Chunk& rhs)
  {
    this->pos_ = rhs.pos_;
    this->storage = rhs.storage;
    return *this;
  }
}