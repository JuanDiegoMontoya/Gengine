#pragma once
#include <span>
#include <vector>
//#include <concepts>

namespace Compression
{
  //template<std::signed_integral T>
  template<typename T>
  std::vector<T> EncodeDelta(std::span<T> array)
  {
    std::vector<T> encoded;
    T prev{ 0 };
    for (T current : array)
    {
      T delta = current - prev;
      prev = current;
      encoded.push_back(delta);
    }
    return encoded;
  }

  //template<std::signed_integral T>
  template<typename T>
  std::vector<T> DecodeDelta(std::span<T> encoded)
  {
    std::vector<T> decoded;
    T prev{ 0 };
    for (T delta : encoded)
    {
      T current = delta + prev;
      prev = current;
      decoded.push_back(current);
    }
    return decoded;
  }
}