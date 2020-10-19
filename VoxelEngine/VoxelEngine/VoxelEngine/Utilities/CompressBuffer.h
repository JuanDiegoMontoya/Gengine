#pragma once
#include <span>
#include <cstddef>
#include <stdint.h>
#include <vector>
#include <zlib.h>
#include <memory>
#include <CoreEngine/GAssert.h>

namespace Compression
{
  // templated for type safety
  template<typename T>
  struct CompressionResult
  {
    uint64_t compressedSize{};
    uint64_t uncompressedSize{};
    std::unique_ptr<std::byte[]> data{};
  };

  template<typename T>
  using UncompressionResult = std::vector<T>;

  // compresses a contiguous set of data
  // data must be serial before compressing
  template<typename T>
  CompressionResult<T> Compress(std::span<T> array)
  {
    CompressionResult<T> compressedData;
    const uint64_t uncompressedSize = array.size_bytes();
    uint64_t compressedSize = uncompressedSize * 1.1 + 12; // zlib-recommended size increase
    auto compressedTemp = std::make_unique<std::byte[]>(compressedSize);
    
    int z_result = compress(
      (Byte*)compressedTemp.get(),
      (uLong*)&compressedSize,
      (Byte*)array.data(),
      uncompressedSize
    );
    assert(z_result == Z_OK);

    // compressedSize may have changed
    compressedData.data = std::make_unique<std::byte[]>(compressedSize);
    std::memcpy(compressedData.data.get(), compressedTemp.get(), compressedSize);
    compressedData.compressedSize = compressedSize;
    compressedData.uncompressedSize = uncompressedSize;

    return compressedData;
  }

  template<typename T>
  UncompressionResult<T> Uncompress(const CompressionResult<T>& compressedData)
  {
    UncompressionResult<T> uncompressedData;
    const uint64_t compressedSize = compressedData.compressedSize;

    uint64_t uncompressedSize = compressedData.uncompressedSize;
    auto uncompressedDataTemp = std::make_unique<std::byte[]>(uncompressedSize);
    int z_result = uncompress(
      (Byte*)uncompressedDataTemp.get(),
      (uLong*)&uncompressedSize,
      (Byte*)compressedData.data.get(),
      compressedSize
    );
    ASSERT(z_result == Z_OK);
    ASSERT(uncompressedSize == compressedData.uncompressedSize);
    ASSERT_MSG(uncompressedSize % sizeof(T) == 0,
      "The uncompressed data size must be a multiple of sizeof(T).");

    uncompressedData.assign((T*)uncompressedDataTemp.get(), (T*)uncompressedDataTemp.get() + uncompressedSize / sizeof(T));
    return uncompressedData;
  }
}