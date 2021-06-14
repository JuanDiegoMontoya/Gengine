#include "CVar.h"
#include "GAssert.h"
#include <unordered_map>
#include <entt.hpp> // TODO: replace with custom hashed string
#include <concepts>
#include <shared_mutex>

template<typename T>
concept Number = std::integral<T> || std::floating_point<T>;

enum class CVarType : uint8_t
{
  INT,
  FLOAT,
  STRING,
  //BOOLEAN,
  //VEC3F,
  //VEC3I,
  //VEC2F,
  //VEC2I,
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
  CVarArray<int, 1000> intCVars;
  CVarArray<float, 1000> floatCVars;
  CVarArray<std::string, 1000, const char*> stringCVars;

  std::unordered_map<entt::id_type, CVarParameters> cvarParameters;
  std::shared_mutex mutex;
};

CVarSystem* CVarSystem::Get()
{
  static CVarSystem system{};
  return &system;
}

CVarSystem::CVarSystem()
{
  storage = new CVarSystemStorage;
}

CVarSystem::~CVarSystem()
{
  delete storage;
}

CVarParameters* CVarSystem::GetCVarParams(const char* name)
{
  if (auto it = storage->cvarParameters.find(entt::hashed_string(name)); it != storage->cvarParameters.end())
  {
    return &it->second;
  }
  return nullptr;
}


CVarParameters* CVarSystem::InitCVar(const char* name, const char* description, CVarFlags flags)
{
  std::unique_lock lck(storage->mutex);
  CVarParameters& params = storage->cvarParameters[entt::hashed_string(name)];
  params.index = -1;
  params.name = name;
  params.description = description;
  params.flags = flags;
  return &params;
}


template<>
CVarParameters* CVarSystem::RegisterCVar<float>(const char* name, const char* description,
  float defaultValue, CVarFlags flags, OnChangeCallback<float> callback)
{
  auto params = InitCVar(name, description, flags);
  params->type = CVarType::FLOAT;
  storage->floatCVars.AddCVar(defaultValue, params, callback);
  return params;
}

template<>
CVarParameters* CVarSystem::RegisterCVar<int32_t>(const char* name, const char* description,
  int32_t defaultValue, CVarFlags flags, OnChangeCallback<int32_t> callback)
{
  auto params = InitCVar(name, description, flags);
  params->type = CVarType::INT;
  storage->intCVars.AddCVar(defaultValue, params, callback);
  return params;
}

template<>
CVarParameters* CVarSystem::RegisterCVar<const char*>(const char* name, const char* description,
  const char* defaultValue, CVarFlags flags, OnChangeCallback<const char*> callback)
{
  auto params = InitCVar(name, description, flags);
  params->type = CVarType::STRING;
  storage->stringCVars.AddCVar(defaultValue, params, callback);
  return params;
}


template<>
float CVarSystem::GetCVar(const char* name)
{
  std::shared_lock lck(storage->mutex);
  if (auto* params = GetCVarParams(name))
  {
    return storage->floatCVars.cvars[params->index].current;
  }
  return 0;
}

template<>
int32_t CVarSystem::GetCVar(const char* name)
{
  std::shared_lock lck(storage->mutex);
  if (auto* params = GetCVarParams(name))
  {
    return storage->intCVars.cvars[params->index].current;
  }
  return 0;
}

template<>
const char* CVarSystem::GetCVar(const char* name)
{
  std::shared_lock lck(storage->mutex);
  if (auto* params = GetCVarParams(name))
  {
    return storage->stringCVars.cvars[params->index].current.c_str();
  }
  return "";
}


template<>
bool CVarSystem::SetCVar(const char* name, float value)
{
  std::unique_lock lck(storage->mutex);
  if (auto* params = GetCVarParams(name))
  {
    auto& cvar = storage->floatCVars.cvars[params->index];
    if (cvar.callback) cvar.callback(name, value);
    cvar.current = value;
    return true;
  }
  return false;
}

template<>
bool CVarSystem::SetCVar(const char* name, int32_t value)
{
  std::unique_lock lck(storage->mutex);
  if (auto* params = GetCVarParams(name))
  {
    auto& cvar = storage->intCVars.cvars[params->index];
    if (cvar.callback) cvar.callback(name, value);
    cvar.current = value;
    return true;
  }
  return false;
}

template<>
bool CVarSystem::SetCVar(const char* name, const char* value)
{
  std::unique_lock lck(storage->mutex);
  if (auto* params = GetCVarParams(name))
  {
    auto& cvar = storage->stringCVars.cvars[params->index];
    if (cvar.callback) cvar.callback(name, value);
    cvar.current = value;
    return true;
  }
  return false;
}


template<>
AutoCVar<float>::AutoCVar(const char* name, const char* description, float defaultValue, CVarFlags flags, OnChangeCallback<float> callback)
{
  auto* params = CVarSystem::Get()->RegisterCVar<float>(name, description, defaultValue, flags, callback);
  index = params->index;
}

template<>
AutoCVar<int>::AutoCVar(const char* name, const char* description, int defaultValue, CVarFlags flags, OnChangeCallback<int> callback)
{
  auto* params = CVarSystem::Get()->RegisterCVar<int>(name, description, defaultValue, flags, callback);
  index = params->index;
}

template<>
AutoCVar<const char*>::AutoCVar(const char* name, const char* description, const char* defaultValue, CVarFlags flags, OnChangeCallback<const char*> callback)
{
  auto* params = CVarSystem::Get()->RegisterCVar<const char*>(name, description, defaultValue, flags, callback);
  index = params->index;
}


template<>
float AutoCVar<float>::Get()
{
  return CVarSystem::Get()->storage->floatCVars.cvars[index].current;
}

template<>
int AutoCVar<int>::Get()
{
  return CVarSystem::Get()->storage->intCVars.cvars[index].current;
}

template<>
const char* AutoCVar<const char*>::Get()
{
  return CVarSystem::Get()->storage->stringCVars.cvars[index].current.c_str();
}


template<>
void AutoCVar<float>::Set(float value)
{
  auto& cvar = CVarSystem::Get()->storage->floatCVars.cvars[index];
  CVarSystem::Get()->SetCVar(cvar.parameters->name.c_str(), value);
}

template<>
void AutoCVar<int>::Set(int value)
{
  auto& cvar = CVarSystem::Get()->storage->floatCVars.cvars[index];
  CVarSystem::Get()->SetCVar(cvar.parameters->name.c_str(), value);
}

template<>
void AutoCVar<const char*>::Set(const char* value)
{
  auto& cvar = CVarSystem::Get()->storage->floatCVars.cvars[index];
  CVarSystem::Get()->SetCVar(cvar.parameters->name.c_str(), value);
}