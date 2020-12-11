#include <CoreEngine/ScriptSystem.h>
#include <CoreEngine/Components.h>

void ScriptSystem::Update(Scene& scene, float dt)
{
  auto view = scene.GetRegistry().view<Components::NativeScriptComponent>();
  for (auto entity : view)
  {
    auto& script = view.get<Components::NativeScriptComponent>(entity);
    if (script.Instance == nullptr)
    {
      script.Instance = script.InstantiateScript();
      script.Instance->entity_ = Entity{ entity, &scene };
      script.Instance->OnCreate();
    }

    script.Instance->OnUpdate(dt);
  }
}
