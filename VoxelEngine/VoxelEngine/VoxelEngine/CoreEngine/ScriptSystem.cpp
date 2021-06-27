#include "PCH.h"
#include <CoreEngine/ScriptSystem.h>
#include <CoreEngine/Scene.h>
#include "Components/Scripting.h"

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

void ScriptSystem::Update(Scene& scene, float dt)
{
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

    script.Instance->OnUpdate(dt);
  }
}
