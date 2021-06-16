#pragma once
#include <cinttypes>
#include "Flags.h"
//#include <glm/fwd.hpp>
//#include <variant>

enum class CVarFlag
{
  // no flags
  NONE = 0,

  // the variable will be saved to a config file
  ARCHIVE = 1 << 0,

  // can only be modified by the user from the command line
  INIT = 1 << 1,

  // cannot be modified by the user
  READ_ONLY = 1 << 2,

  // created by a `set` command
  USER_CREATED = 1 << 3,

  // can only be changed if cheats are enabled
  CHEAT = 1 << 4,

  //USER_INFO = 1 << 1,
  //SERVER_INFO = 1 << 2,
  //SYSTEM_INFO = 1 << 3,
  //LATCH = 1 << 5,
  //TEMP = 1 << 8,
  //NORESTART = 1 << 10,
};
DECLARE_FLAG_TYPE(CVarFlags, CVarFlag, uint32_t)

template<typename T>
using OnChangeCallback = void(*)(const char*, T);

struct CVarSystemStorage;
struct CVarParameters;
class CVarSystem
{
public:
  static CVarSystem* Get();
  ~CVarSystem();

  template<typename T>
  CVarParameters* RegisterCVar(const char* name, const char* description, T defaultValue, CVarFlags flags = CVarFlag::NONE, OnChangeCallback<T> callback = nullptr);

  template<typename T>
  T GetCVar(const char* name);

  template<typename T>
  bool SetCVar(const char* name, T value);

private:
  template<typename U>
  friend class AutoCVar;

  CVarSystem();
  CVarParameters* InitCVar(const char* name, const char* description, CVarFlags flags);
  CVarParameters* GetCVarParams(const char* name);
  
  CVarSystemStorage* storage{};
};

template<typename T>
class AutoCVar
{
public:
  AutoCVar(const char* name, const char* description, T defaultValue, CVarFlags flags = CVarFlag::NONE, OnChangeCallback<T> callback = nullptr);

  T Get();
  void Set(T);

private:
  int index{};
};

template<> CVarParameters* CVarSystem::RegisterCVar(const char*, const char*, float, CVarFlags, OnChangeCallback<float>);
template<> CVarParameters* CVarSystem::RegisterCVar(const char*, const char*, int, CVarFlags, OnChangeCallback<int32_t>);
template<> CVarParameters* CVarSystem::RegisterCVar(const char*, const char*, const char*, CVarFlags, OnChangeCallback<const char*>);

template<> float CVarSystem::GetCVar(const char*);
template<> int32_t CVarSystem::GetCVar(const char*);
template<> const char* CVarSystem::GetCVar(const char*);

template<> bool CVarSystem::SetCVar(const char*, float);
template<> bool CVarSystem::SetCVar(const char*, int32_t);
template<> bool CVarSystem::SetCVar(const char*, const char*);


template<> AutoCVar<float>::AutoCVar(const char*, const char*, float, CVarFlags, OnChangeCallback<float>);
template<> AutoCVar<int>::AutoCVar(const char*, const char*, int, CVarFlags, OnChangeCallback<int>);
template<> AutoCVar<const char*>::AutoCVar(const char*, const char*, const char*, CVarFlags, OnChangeCallback<const char*>);

template<> float AutoCVar<float>::Get();
template<> int AutoCVar<int>::Get();
template<> const char* AutoCVar<const char*>::Get();

template<> void AutoCVar<float>::Set(float);
template<> void AutoCVar<int>::Set(int);
template<> void AutoCVar<const char*>::Set(const char*);