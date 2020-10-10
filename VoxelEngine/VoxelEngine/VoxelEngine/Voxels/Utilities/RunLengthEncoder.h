#pragma once
#include <span>
#include <vector>
#include <engine_assert.h>

template<typename T>
struct RLEelement
{
  uint32_t count{};
  T value{};
};

template<typename T>
std::vector<RLEelement<T>> EncodeRLE(std::span<T> array)
{
  std::vector<RLEelement<T>> data;

  RLEelement curElement{ .value = array[0] };
  for (auto value : array)
  {
    if (value != curElement.value)
    {
      data.push_back(curElement);
      curElement = RLEelement{ .count = 0, .value = value };
    }
    else
    {
      curElement.count++;
    }
  }

  return data;
}

template<typename T>
std::vector<T> DecodeRLE(std::vector<RLEelement<T>> data)
{
  std::vector<T> array;

  for (auto rlee : data)
  {
    array.insert(array.end(), rlee.count, rlee.value);
  }

  return array;
}