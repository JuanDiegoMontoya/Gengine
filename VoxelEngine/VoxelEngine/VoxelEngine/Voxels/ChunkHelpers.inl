#pragma once
#include <Voxels/ChunkHelpers.h>
//#include <Chunks/ChunkHelpers.h>
//#include <Chunks/Chunk.h>

namespace Voxels
{
  namespace ChunkHelpers
  {
    inline localpos WorldPosToLocalPos(const glm::ivec3& wpos)
    {
      localpos ret;
      WorldPosToLocalPosFast(wpos, ret);
      return ret;
    }

    inline void WorldPosToLocalPosFast(const glm::ivec3& wpos, localpos& ret)
    {
      // TODO: this is a hack to make the compiler happy
      constexpr int CHUNK_SIZE = 32;
      constexpr int CHUNK_SIZE_LOG2 = 5;

      // compute the modulus of wpos and chunk size (bitwise AND method only works for powers of 2)
      // to find the relative block position in the chunk
      //ret.block_pos.x = wpos.x & CHUNK_SIZE - 1;
      //ret.block_pos.y = wpos.y & CHUNK_SIZE - 1;
      //ret.block_pos.z = wpos.z & CHUNK_SIZE - 1;
      ret.block_pos = wpos & CHUNK_SIZE - 1;
      //// find the chunk position using integer floor method
      //ret.chunk_pos.x = wpos.x / CHUNK_SIZE;
      //ret.chunk_pos.y = wpos.y / CHUNK_SIZE;
      //ret.chunk_pos.z = wpos.z / CHUNK_SIZE;
      //// subtract (floor) if negative w/ non-zero modulus
      //if (wpos.x < 0 && ret.block_pos.x) ret.chunk_pos.x--;
      //if (wpos.y < 0 && ret.block_pos.y) ret.chunk_pos.y--;
      //if (wpos.z < 0 && ret.block_pos.z) ret.chunk_pos.z--;
      ret.chunk_pos = wpos >> CHUNK_SIZE_LOG2;
      // shift local block position forward by chunk size if negative
      //if (ret.block_pos.x < 0) ret.block_pos.x += CHUNK_SIZE;
      //if (ret.block_pos.y < 0) ret.block_pos.y += CHUNK_SIZE;
      //if (ret.block_pos.z < 0) ret.block_pos.z += CHUNK_SIZE;
    }

    inline glm::ivec3 LocalPosToWorldPos(const glm::ivec3& local, const glm::ivec3& cpos)
    {
      // TODO: this is a hack to make the compiler happy
      constexpr int CHUNK_SIZE = 32;
      return glm::ivec3(local + (cpos * CHUNK_SIZE));
    }
  }
}