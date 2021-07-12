#pragma once

namespace Voxels
{
  namespace ChunkHelpers
  {
    struct localpos
    {
      localpos() : chunk_pos(0), block_pos(0) {}
      localpos(const glm::ivec3& chunk, const glm::ivec3& block)
        : chunk_pos(chunk), block_pos(block)
      {
      }
      localpos(glm::ivec3&& chunk, glm::ivec3&& block)
        : chunk_pos(std::move(chunk)), block_pos(std::move(block))
      {
      }
      glm::ivec3 chunk_pos; // within world
      glm::ivec3 block_pos; // within chunk
    };

    localpos WorldPosToLocalPos(const glm::ivec3& wpos);
    void WorldPosToLocalPosFast(const glm::ivec3& wpos, localpos& ret);
    inline glm::ivec3 LocalPosToWorldPos(const glm::ivec3& local, const glm::ivec3& cpos);

    auto IndexFrom3D(auto x, auto y, auto z, auto h, auto w)
    {
      return x + h * (y + w * z);
    }

    auto IndexFrom2D(auto x, auto y, auto w)
    {
      return w * y + x;
    }
  }
}

#include "ChunkHelpers.inl"