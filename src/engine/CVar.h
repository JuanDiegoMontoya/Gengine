#pragma once
#include <cstdint>
#include <utility/Flags.h>
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

  CVarParameters* RegisterCVar(const char* name,
    const char* description,
    cvar_string defaultValue,
    CVarFlags flags = CVarFlag::NONE,
    OnChangeCallback<cvar_string> callback = nullptr);

  CVarParameters* RegisterCVar(const char* name,
    const char* description,
    cvar_float defaultValue,
    cvar_float minValue = cvar_float{},
    cvar_float maxValue = cvar_float{},
    CVarFlags flags = CVarFlag::NONE,
    OnChangeCallback<cvar_float> callback = nullptr);

  CVarParameters* RegisterCVar(const char* name,
    const char* description,
    cvar_vec3 defaultValue,
    cvar_vec3 minValue = cvar_vec3{},
    cvar_vec3 maxValue = cvar_vec3{},
    CVarFlags flags = CVarFlag::NONE,
    OnChangeCallback<cvar_vec3> callback = nullptr);

  template<typename T>
  T GetCVar(const char* name);

  template<typename T>
  bool SetCVar(const char* name, T value);

  bool SetCVarParse(const char* name, const char* args);

private:
  template<typename U>
  friend class AutoCVarBase;

  friend class Console;

  CVarSystem();
  CVarParameters* InitCVar(const char* name, const char* description, CVarFlags flags);
  CVarParameters* GetCVarParams(const char* name);

  CVarSystemStorage* storage{};
};

template<typename T>
class AutoCVarBase
{
public:
  AutoCVarBase(AutoCVarBase&) = delete;
  AutoCVarBase& operator=(AutoCVarBase&) = delete;

  T Get();
  void Set(T value);

protected:
  AutoCVarBase() = default;

  int index{};
};

template<typename T>
class AutoCVar final : public AutoCVarBase<T>
{
public:
  AutoCVar(const char* name, 
    const char* description, 
    T defaultValue, 
    CVarFlags flags = CVarFlag::NONE, 
    OnChangeCallback<T> callback = nullptr);
};

template<>
class AutoCVar<cvar_float> final : public AutoCVarBase<cvar_float>
{
public:
  AutoCVar(const char* name,
    const char* description,
    cvar_float defaultValue,
    cvar_float minValue = cvar_float{},
    cvar_float maxValue = cvar_float{},
    CVarFlags flags = CVarFlag::NONE,
    OnChangeCallback<cvar_float> callback = nullptr);
};

template<>
class AutoCVar<cvar_vec3> final : public AutoCVarBase<cvar_vec3>
{
public:
  AutoCVar(const char* name,
    const char* description,
    cvar_vec3 defaultValue,
    cvar_vec3 minValue = cvar_vec3{},
    cvar_vec3 maxValue = cvar_vec3{},
    CVarFlags flags = CVarFlag::NONE,
    OnChangeCallback<cvar_vec3> callback = nullptr);
};

//template<> CVarParameters* CVarSystem::RegisterCVar(const char*, const char*, cvar_float, cvar_float, cvar_float, CVarFlags, OnChangeCallback<cvar_float>);
//template<> CVarParameters* CVarSystem::RegisterCVar(const char*, const char*, cvar_string, CVarFlags, OnChangeCallback<cvar_string>);
//template<> CVarParameters* CVarSystem::RegisterCVar(const char*, const char*, cvar_vec3, cvar_vec3, cvar_vec3, CVarFlags, OnChangeCallback<cvar_vec3>);

template<> cvar_float CVarSystem::GetCVar(const char*);
template<> cvar_string CVarSystem::GetCVar(const char*);
template<> cvar_vec3 CVarSystem::GetCVar(const char*);

template<> bool CVarSystem::SetCVar(const char*, cvar_float);
template<> bool CVarSystem::SetCVar(const char*, cvar_string);
template<> bool CVarSystem::SetCVar(const char*, cvar_vec3);


//template<> AutoCVar<cvar_float>::AutoCVar(const char*, const char*, cvar_float, cvar_float, cvar_float, CVarFlags, OnChangeCallback<cvar_float>);
template<> AutoCVar<cvar_string>::AutoCVar(const char*, const char*, cvar_string, CVarFlags, OnChangeCallback<cvar_string>);
//template<> AutoCVar<cvar_vec3>::AutoCVar(const char*, const char*, cvar_vec3, cvar_vec3, cvar_vec3, CVarFlags, OnChangeCallback<cvar_vec3>);

template<> cvar_float AutoCVarBase<cvar_float>::Get();
template<> cvar_string AutoCVarBase<cvar_string>::Get();
template<> cvar_vec3 AutoCVarBase<cvar_vec3>::Get();

template<> void AutoCVarBase<cvar_float>::Set(cvar_float);
template<> void AutoCVarBase<cvar_string>::Set(cvar_string);
template<> void AutoCVarBase<cvar_vec3>::Set(cvar_vec3);