#include "Scene.h"
#include "Entity.h"
#include "Components.h"

Scene::Scene(std::string_view name, Engine& engine)
  : name_(name), engine_(engine)
{
}

Scene::~Scene()
{
}

Entity Scene::CreateEntity(std::string_view name)
{
  Entity entity(registry_.create(), this);
  auto& tag = entity.AddComponent<Components::Tag>();
  tag.tag = name.empty() ? "Entity" : name;
  return entity;
}

void Scene::RemoveEntity(std::string_view name)
{
}

void Scene::RemoveEntity(Entity entity)
{
}
