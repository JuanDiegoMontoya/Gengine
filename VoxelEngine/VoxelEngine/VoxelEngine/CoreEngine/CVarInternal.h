#pragma once
#include <concepts>
#include <shared_mutex>
#include <unordered_map>
#include <Utilities/HashedString.h>
#include <string>
#include <glm/vec3.hpp>
#include "GAssert.h"
#include "CVar.h"

template<typename T>
concept Number = std::integral<T> || std::floating_point<T>;

enum class CVarType : uint8_t
{
  FLOAT,
  STRING,
  VEC3,
  //BOOLEAN,
  //VEC2,
};

struct CVarParameters
{
  int index{};
  CVarType type{};
  CVarFlags flags{};
  std::string name;
  std::string description;
};

template<typename T, typename CB = T>
struct CVarStorage
{
  T initial{};
  T current{};
  CVarParameters* parameters{};
  OnChangeCallback<CB> callback{};
};

// numeric cvars can have a min and max imposed upon the user
template<Number T>
struct CVarStorage<T, T>
{
  T initial{};
  T current{};
  T min{};
  T max{};
  CVarParameters* parameters{};
  OnChangeCallback<T> callback{};
};

template<typename T, int Capacity, typename CB = T>
struct CVarArray
{
  CVarStorage<T, CB>* cvars;
  std::atomic_int nextIndex{ 0 };

  CVarArray()
  {
    cvars = new CVarStorage<T, CB>[Capacity];
  }

  ~CVarArray()
  {
    delete[] cvars;
  }

  int AddCVar(const T& value, CVarParameters* params, OnChangeCallback<CB> callback)
  {
    int index = nextIndex++;
    ASSERT_MSG(index < Capacity, "CVar count exceeds storage capacity");

    cvars[index].initial = value;
    cvars[index].current = value;
    cvars[index].parameters = params;
    cvars[index].callback = callback;

    params->index = index;
    return index;
  }
};

struct CVarSystemStorage
{
  CVarArray<cvar_float, 1000> floatCVars;
  CVarArray<std::string, 1000, const char*> stringCVars;
  CVarArray<cvar_vec3, 1000> vec3CVars;

  std::unordered_map<uint32_t, CVarParameters> cvarParameters;
  std::shared_mutex mutex;
};