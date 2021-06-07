#include "EnginePCH.h"
#include <CoreEngine/LifetimeSystem.h>
#include <CoreEngine/Scene.h>
#include "Components/Core.h"
#include "Entity.h"

void LifetimeSystem::Update(Scene& scene, float dt)
{
  auto lifeview = scene.GetRegistry().view<Component::Lifetime>();
  for (auto entity : lifeview)
  {
    auto& lifetime = lifeview.get<Component::Lifetime>(entity);

    if (lifetime.active)
    {
      lifetime.remainingSeconds -= dt;
      if (lifetime.remainingSeconds <= 0)
      {
        Entity ent(entity, &scene);
        ent.Destroy();
      }
    }
  }

  auto deleteview = scene.GetRegistry().view<Component::ScheduledDeletion>();
  for (auto entity : deleteview)
  {
    //printf("Killing entity %d\n", entity);
    scene.GetRegistry().destroy(entity);
  }
}
