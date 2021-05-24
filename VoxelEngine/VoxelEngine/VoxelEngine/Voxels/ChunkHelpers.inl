#pragma once
#include <Voxels/ChunkHelpers.h>
//#include <Chunks/ChunkHelpers.h>
//#include <Chunks/Chunk.h>

namespace Voxels
{
  namespace ChunkHelpers
  {
    inline localpos worldPosToLocalPos(const glm::ivec3& wpos)
    {
      localpos ret;
      fastWorldPosToLocalPos(wpos, ret);
      return ret;
    }

    inline void fastWorldPosToLocalPos(const glm::ivec3& wpos, localpos& ret)
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

    inline glm::ivec3 chunkPosToWorldPos(const glm::ivec3& local, const glm::ivec3& cpos)
    {
      // TODO: this is a hack to make the compiler happy
      constexpr int CHUNK_SIZE = 32;
      return glm::ivec3(local + (cpos * CHUNK_SIZE));
    }

    // counterclockwise from bottom right texture coordinates
    inline const glm::vec2 tex_corners[] =
    {
      { 1, 0 },
      { 1, 1 },
      { 0, 1 },
      { 0, 0 },
    };

    inline void Decode(GLuint encoded, glm::uvec3& modelPos, glm::vec3& normal, glm::vec3& texCoord)
    {
      // decode vertex position
      modelPos.x = encoded >> 26;
      modelPos.y = (encoded >> 20) & 0x3F; // = 0b111111
      modelPos.z = (encoded >> 14) & 0x3F; // = 0b111111

      // decode normal
      GLuint normalIdx = (encoded >> 11) & 0x7; // = 0b111
      ASSERT(normalIdx <= 5);
      if (normalIdx <= 5)
        normal = faces[normalIdx];

      // decode texture index and UV
      GLuint textureIdx = (encoded >> 2) & 0x1FF; // = 0b1111111111
      GLuint cornerIdx = (encoded >> 0) & 0x3; // = 0b11

      texCoord = glm::vec3(tex_corners[cornerIdx], textureIdx);
    }


    inline GLuint EncodeVertex(const glm::uvec3& modelPos, GLuint normalIdx, GLuint texIdx, GLuint cornerIdx)
    {
      GLuint encoded = 0;

      // encode vertex position
      encoded |= modelPos.x << 26;
      encoded |= modelPos.y << 20;
      encoded |= modelPos.z << 14;

      // encode normal
      encoded |= normalIdx << 11;

      // encode texture information
      encoded |= texIdx << 2;
      encoded |= cornerIdx << 0;

#if 0 // debug
      glm::uvec3 mdl;
      glm::vec3 nml, txc;
      Decode(encoded, mdl, nml, txc);
      ASSERT(mdl == modelPos);
      ASSERT(texIdx == txc.z);
      ASSERT(faces[normalIdx] == nml)
#endif

        return encoded;
    }


    // packs direction to center of block with lighting information
    inline uint32_t EncodeLight(uint32_t lightCoding, glm::ivec3 dirCent, uint32_t ao)
    {
      uint32_t encoded = lightCoding;
      dirCent = (dirCent + 1);
      //printf("(%d, %d, %d)\n", dirCent.x, dirCent.y, dirCent.z);

      using namespace glm;
      ASSERT(all(greaterThanEqual(dirCent, ivec3(0, 0, 0))) &&
        all(lessThanEqual(dirCent, ivec3(1, 1, 1))) &&
        ao <= 3);

      encoded |= ao << 19;

      encoded |= dirCent.x << 18;
      encoded |= dirCent.y << 17;
      encoded |= dirCent.z << 16;

      return encoded;
    }
  }
}