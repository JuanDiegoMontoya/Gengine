#pragma once
#include <cstdint>
#include "Flags.h"
#include <glm/fwd.hpp>
#include <functional>

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
using OnChangeCallback = std::function<void(const char*, T)>;

using cvar_float = double;
using cvar_string = const char*;
using cvar_vec3 = glm::vec3;

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

  bool SetCVarParse(const char* name, const char* args);

private:
  template<typename U>
  friend class AutoCVar;

  friend class Console;

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
  
  AutoCVar(AutoCVar&) = delete;
  AutoCVar(AutoCVar&&) = delete;
  AutoCVar& operator=(AutoCVar&) = delete;
  AutoCVar& operator=(AutoCVar&&) = delete;

  T Get();
  void Set(T value);

private:

  int index{};
};

template<> CVarParameters* CVarSystem::RegisterCVar(const char*, const char*, cvar_float, CVarFlags, OnChangeCallback<cvar_float>);
template<> CVarParameters* CVarSystem::RegisterCVar(const char*, const char*, cvar_string, CVarFlags, OnChangeCallback<cvar_string>);
template<> CVarParameters* CVarSystem::RegisterCVar(const char*, const char*, cvar_vec3, CVarFlags, OnChangeCallback<cvar_vec3>);

template<> cvar_float CVarSystem::GetCVar(const char*);
template<> cvar_string CVarSystem::GetCVar(const char*);
template<> cvar_vec3 CVarSystem::GetCVar(const char*);

template<> bool CVarSystem::SetCVar(const char*, cvar_float);
template<> bool CVarSystem::SetCVar(const char*, cvar_string);
template<> bool CVarSystem::SetCVar(const char*, cvar_vec3);


template<> AutoCVar<cvar_float>::AutoCVar(const char*, const char*, cvar_float, CVarFlags, OnChangeCallback<cvar_float>);
template<> AutoCVar<cvar_string>::AutoCVar(const char*, const char*, cvar_string, CVarFlags, OnChangeCallback<cvar_string>);
template<> AutoCVar<cvar_vec3>::AutoCVar(const char*, const char*, cvar_vec3, CVarFlags, OnChangeCallback<cvar_vec3>);

template<> cvar_float AutoCVar<cvar_float>::Get();
template<> cvar_string AutoCVar<cvar_string>::Get();
template<> cvar_vec3 AutoCVar<cvar_vec3>::Get();

template<> void AutoCVar<cvar_float>::Set(cvar_float);
template<> void AutoCVar<cvar_string>::Set(cvar_string);
template<> void AutoCVar<cvar_vec3>::Set(cvar_vec3);