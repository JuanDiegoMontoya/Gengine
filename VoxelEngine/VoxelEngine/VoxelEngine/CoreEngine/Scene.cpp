#include "Scene.h"
#include "Entity.h"
#include "Components.h"

Entity Scene::CreateEntity(std::string_view name)
{
  Entity entity(registry_.create(), this);
  auto& tag = entity.AddComponent<Tag>();
  tag.tag = name.empty() ? "Entity" : name;
  return entity;
}

void Scene::RemoveEntity(std::string_view name)
{
}

void Scene::RemoveEntity(Entity entity)
{
}
