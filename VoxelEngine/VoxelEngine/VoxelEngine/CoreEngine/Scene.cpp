#include "EnginePCH.h"
#include "Scene.h"
#include "Entity.h"
#include "Components/Core.h"
#include "Components/Transform.h"

Scene::Scene(std::string_view name, Engine& engine)
  : name_(name), engine_(engine)
{
}

Scene::~Scene()
{
  registry_.clear();
}

Entity Scene::CreateEntity(std::string_view name)
{
  Entity entity(registry_.create(), this);
  auto& tag = entity.AddComponent<Component::Tag>();
  tag.tag = name.empty() ? "Entity" : name;
  return entity;
}

Entity Scene::GetEntity(std::string_view name)
{
  auto view = registry_.view<Component::Tag>();
  for (entt::entity entity : view)
  {
    const auto& tag = view.get<Component::Tag>(entity);
    if (tag.tag == name)
    {
      return Entity(entity, this);
      break;
    }
  }

  return Entity(entt::null, this);
  // Log("No entity called "%s" exists to return", name.c_str());
}
