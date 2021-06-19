#include "CVarInternal.h"

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
CVarParameters* CVarSystem::RegisterCVar<double>(const char* name, const char* description,
  double defaultValue, CVarFlags flags, OnChangeCallback<double> callback)
{
  auto params = InitCVar(name, description, flags);
  params->type = CVarType::FLOAT;
  storage->floatCVars.AddCVar(defaultValue, params, callback);
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
cvar_float CVarSystem::GetCVar(const char* name)
{
  std::shared_lock lck(storage->mutex);
  if (auto* params = GetCVarParams(name))
  {
    return storage->floatCVars.cvars[params->index].current;
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
bool CVarSystem::SetCVar(const char* name, double value)
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
AutoCVar<double>::AutoCVar(const char* name, const char* description, double defaultValue, CVarFlags flags, OnChangeCallback<double> callback)
{
  auto* params = CVarSystem::Get()->RegisterCVar(name, description, defaultValue, flags, callback);
  index = params->index;
}

template<>
AutoCVar<const char*>::AutoCVar(const char* name, const char* description, const char* defaultValue, CVarFlags flags, OnChangeCallback<const char*> callback)
{
  auto* params = CVarSystem::Get()->RegisterCVar(name, description, defaultValue, flags, callback);
  index = params->index;
}


template<>
double AutoCVar<double>::Get()
{
  return CVarSystem::Get()->storage->floatCVars.cvars[index].current;
}

template<>
const char* AutoCVar<const char*>::Get()
{
  return CVarSystem::Get()->storage->stringCVars.cvars[index].current.c_str();
}


template<>
void AutoCVar<double>::Set(double value)
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