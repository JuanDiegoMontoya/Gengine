#include "../../PCH.h"
#include "LifetimeSystem.h"
#include "../../Scene.h"
#include "../component/Core.h"
#include "../component/Transform.h"
#include "../Entity.h"

static void DeleteEntityAndChildren(Entity entity, std::vector<entt::entity>& entitiesToDelete)
{
  entitiesToDelete.push_back(entity);
  printf("Killing entity %u, %s\n", uint32_t(entity), entity.GetComponent<Component::Tag>().tag.c_str());

  // DFS child removal
  if (entity.HasComponent<Component::Children>())
  {
    auto& children = entity.GetComponent<Component::Children>();
    for (auto child : children.GetChildren())
    {
      DeleteEntityAndChildren(child, entitiesToDelete);
    }
  }
}

void LifetimeSystem::Update(Scene& scene, Timestep timestep)
{
  auto lifeview = scene.GetRegistry().view<Component::Lifetime>();
  for (auto entity : lifeview)
  {
    auto& lifetime = lifeview.get<Component::Lifetime>(entity);

    if (lifetime.active)
    {
      lifetime.remainingSeconds -= timestep.dt_effective;
      if (lifetime.remainingSeconds <= 0)
      {
        Entity ent(entity, &scene);
        ent.Destroy();
      }
    }
  }

  auto deleteview = scene.GetRegistry().view<Component::ScheduledDeletion>();
  std::vector<entt::entity> entitiesToDelete;
  entitiesToDelete.reserve(deleteview.size());
  for (auto ent : deleteview)
  {
    Entity entity(ent, &scene);

    DeleteEntityAndChildren(entity, entitiesToDelete);
  }

  for (auto entity : entitiesToDelete)
  {
    scene.GetRegistry().destroy(entity);
  }
}
