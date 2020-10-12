#pragma once
#include <span>
#include <cstddef>
#include <stdint.h>
#include <vector>
#include <zlib.h>
#include <memory>

#pragma optimize("", off)
namespace Compression
{
  struct CompressionResult
  {
    uint64_t sizeBytes{};
    std::unique_ptr<std::byte[]> data{};
  };

  template<typename T>
  struct UncompressionResult
  {
    uint64_t count{}; // element COUNT
    std::unique_ptr<T[]> data{};
  };

  template<typename T>
  CompressionResult Compress(std::span<T> array)
  {
    CompressionResult compressedData;
    const uint64_t uncompressedSize = array.size_bytes();
    uint64_t compressedSize = uncompressedSize * 1.1 + 12; // zlib-recommended size increase
    auto compressedTemp = std::make_unique<std::byte[]>(compressedSize);
    int z_result = compress(
      (Byte*)compressedTemp.get(),
      (uLong*)&compressedSize,
      (Byte*)array.data(),
      uncompressedSize
    );
    ASSERT(z_result == Z_OK);

    // compressedSize may have changed
    compressedData.data = std::make_unique<std::byte[]>(compressedSize);
    std::memcpy(compressedData.data.get(), compressedTemp.get(), compressedSize);

    return compressedData;
  }

  template<typename T>
  UncompressionResult<T> Uncompress(std::span<std::byte> compressedData)
  {
    // TODO: impl this func
  }
}