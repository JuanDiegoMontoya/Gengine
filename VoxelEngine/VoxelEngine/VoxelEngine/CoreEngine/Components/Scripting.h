#pragma once
#include "../ScriptableEntity.h"
#include <functional>

namespace Components
{
  struct NativeScriptComponent
  {
    ScriptableEntity* Instance = nullptr;

    std::function<ScriptableEntity* ()> InstantiateScript;
    void (*DestroyScript)(NativeScriptComponent*);

    template<typename T, typename ...Args>
    void Bind(Args&&... args)
    {
      // perfectly forwards arguments to function object with which to instantiate this script
      InstantiateScript = [... args = std::forward<Args>(args)]() mutable
        {
          return static_cast<ScriptableEntity*>(new T(std::forward<Args>(args)...));
        };
      DestroyScript = [](NativeScriptComponent* nsc) { delete nsc->Instance; nsc->Instance = nullptr; };
    }
  };
}