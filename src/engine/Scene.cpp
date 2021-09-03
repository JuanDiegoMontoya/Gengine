#include "PCH.h"
#include <entt/entt.hpp>
#include "Scene.h"
#include "ecs/Entity.h"
#include "ecs/component/Core.h"
#include "ecs/component/Transform.h"

Scene::Scene(std::string_view name, Engine* engine)
  : name_(name), engine_(engine)
{
  registry_ = new entt::registry;
}

Scene::~Scene()
{
  registry_->clear();
  delete registry_;
}

Entity Scene::CreateEntity(std::string_view name)
{
  Entity entity(registry_->create(), this);
  auto& tag = entity.AddComponent<Component::Tag>();
  tag.tag = name.empty() ? "Entity" : name;
  return entity;
}

Entity Scene::GetEntity(std::string_view name)
{
  auto view = registry_->view<Component::Tag>();
  for (entt::entity entity : view)
  {
    const auto& tag = view.get<Component::Tag>(entity);
    if (tag.tag == name)
    {
      return Entity(entity, this);
      break;
    }
  }

  return Entity{};
  //return Entity(entt::null, this);
  // Log("No entity called "%s" exists to return", name.c_str());
}
