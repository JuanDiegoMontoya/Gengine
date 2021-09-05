#include "../../PCH.h"
#include "../../Scene.h"
#include "../component/Scripting.h"
#include "ScriptSystem.h"

#include <engine/core/StatMacros.h>

DECLARE_FLOAT_STAT(ScriptUpdate, CPU)

static void OnScriptDestroy(entt::basic_registry<entt::entity>& registry, entt::entity entity)
{
  auto& script = registry.get<Component::NativeScriptComponent>(entity);
  script.DestroyScript(&script);
}

void ScriptSystem::InitScene(Scene& scene)
{
  scene.GetRegistry().on_destroy<Component::NativeScriptComponent>()
    .connect<&OnScriptDestroy>();
}

void ScriptSystem::Update(Scene& scene, Timestep timestep)
{
  MEASURE_CPU_TIMER_STAT(ScriptUpdate);

  auto view = scene.GetRegistry().view<Component::NativeScriptComponent>();
  for (auto entity : view)
  {
    auto& script = view.get<Component::NativeScriptComponent>(entity);
    if (script.Instance == nullptr)
    {
      script.Instance = script.InstantiateScript();
      script.Instance->entity_ = Entity{ entity, &scene };
      script.Instance->OnCreate();
    }

    script.Instance->OnUpdate(timestep);
  }
}
