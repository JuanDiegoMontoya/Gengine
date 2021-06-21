#pragma once
#include <concepts>
#include <shared_mutex>
#include <unordered_map>
#include <entt.hpp> // TODO: replace with custom hashed string
#include <string>
#include "GAssert.h"
#include "CVar.h"

template<typename T>
concept Number = std::integral<T> || std::floating_point<T>;

enum class CVarType : uint8_t
{
  FLOAT,
  STRING,
  //BOOLEAN,
  //VEC3,
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
  CVarArray<double, 1000> floatCVars;
  CVarArray<std::string, 1000, const char*> stringCVars;

  std::unordered_map<entt::id_type, CVarParameters> cvarParameters;
  std::shared_mutex mutex;
};