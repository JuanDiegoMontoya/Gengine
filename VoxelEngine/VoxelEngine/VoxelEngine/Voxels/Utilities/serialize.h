#pragma once
#include <glm/glm.hpp>
#include <Utilities/CompressBuffer.h>
#include <Utilities/RunLengthEncoder.h>
#include <Utilities/DeltaEncoder.h>
#include <cereal/types/vector.hpp>
#include <cereal/archives/binary.hpp>

struct Chunk;

namespace cereal
{
  template<class Archive>
  void serialize(Archive& archive, glm::vec3& v)
  {
    archive(v.x, v.y, v.z);
  }

  template<class Archive>
  void serialize(Archive& archive, glm::ivec3& v)
  {
    archive(v.x, v.y, v.z);
  }

  template<class Archive>
  void serialize(Archive& archive, Chunk* c)
  {
    if (c)
      archive(*c);
  }

  template<class Archive, typename T>
  void serialize(Archive& archive, Compression::RLEelement<T>& elem)
  {
    archive(elem.count, elem.value);
  }

  template<class Archive, typename T>
  void save(Archive& archive, const Compression::CompressionResult<T>& comp)
  {
    archive(
      comp.compressedSize, 
      comp.uncompressedSize, 
      cereal::binary_data(comp.data.get(), comp.compressedSize * sizeof(std::byte)));
  }
  template<class Archive, typename T>
  void load(Archive& archive, Compression::CompressionResult<T>& comp)
  {
    archive(comp.compressedSize);
    comp.data = std::make_unique<std::byte[]>(comp.compressedSize);
    archive(comp.uncompressedSize,
      cereal::binary_data(comp.data.get(), comp.compressedSize * sizeof(std::byte)));
  }
}