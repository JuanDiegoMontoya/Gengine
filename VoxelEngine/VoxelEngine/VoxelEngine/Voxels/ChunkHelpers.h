#pragma once

namespace ChunkHelpers
{
  struct localpos
  {
    localpos() : chunk_pos(0), block_pos(0) {}
    localpos(const glm::ivec3& chunk, const glm::ivec3& block)
      : chunk_pos(chunk), block_pos(block) {}
    localpos(glm::ivec3&& chunk, glm::ivec3&& block)
      : chunk_pos(std::move(chunk)), block_pos(std::move(block)) {}
    glm::ivec3 chunk_pos; // within world
    glm::ivec3 block_pos; // within chunk
  };

  localpos worldPosToLocalPos(const glm::ivec3& wpos);
  void fastWorldPosToLocalPos(const glm::ivec3& wpos, localpos& ret);
  inline glm::ivec3 chunkPosToWorldPos(const glm::ivec3& local, const glm::ivec3& cpos);

  GLuint EncodeVertex(const glm::uvec3& modelPos, GLuint normalIdx, GLuint texIdx, GLuint cornerIdx);
  void Decode(GLuint encoded, glm::uvec3& modelPos, glm::vec3& normal, glm::vec2& texCoord);

  GLuint EncodeSplat(const glm::uvec3& modelPos, const glm::vec3& color);

  auto IndexFrom3D(auto x, auto y, auto z, auto h, auto w)
  {
    return x + h * (y + w * z);
  }

  auto IndexFrom2D(auto x, auto y, auto w)
  {
    return w * y + x;
  }

  inline const glm::ivec3 faces[6] =
  {
    { 0, 0, 1 }, // 'far' face    (+z direction)
    { 0, 0,-1 }, // 'near' face   (-z direction)
    {-1, 0, 0 }, // 'left' face   (-x direction)
    { 1, 0, 0 }, // 'right' face  (+x direction)
    { 0, 1, 0 }, // 'top' face    (+y direction)
    { 0,-1, 0 }, // 'bottom' face (-y direction)
  };
}

#include "ChunkHelpers.inl"