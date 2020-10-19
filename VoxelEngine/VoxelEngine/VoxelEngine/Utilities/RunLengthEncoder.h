#pragma once
#include <span>
#include <vector>

namespace Compression
{
  template<typename T>
  struct RLEelement
  {
    bool operator==(const RLEelement<T> rhs) const
    {
      return count == rhs.count && value == rhs.value;
    }
    uint32_t count{};
    T value{};
  };

  template<typename T>
  std::vector<RLEelement<T>> EncodeRLE(std::span<T> array)
  {
    std::vector<RLEelement<T>> data;

    RLEelement<T> curElement{ .value = array[0] };
    for (auto value : array)
    {
      if (value != curElement.value)
      {
        data.push_back(curElement);
        curElement = RLEelement<T>{ .count = 0, .value = value };
      }
      curElement.count++;
    }
    if (curElement.count > 0)
      data.push_back(curElement);

    return data;
  }

  template<typename T>
  std::vector<T> DecodeRLE(std::span<RLEelement<T>> data)
  {
    std::vector<T> array;

    for (auto rlee : data)
    {
      array.insert(array.end(), rlee.count, rlee.value);
    }

    return array;
  }
}