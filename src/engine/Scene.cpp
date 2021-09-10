#include "PCH.h"
#include "Scene.h"
#include <entt/entt.hpp>
#include <unordered_map>
#include "ecs/Entity.h"
#include "ecs/component/Core.h"
#include "ecs/component/Transform.h"

struct SceneStorage
{
  entt::registry registry_{};  // all the entities in this scene
  Engine* engine_{};           // non-owning
  std::string name_;           // the name of this scene
  std::unordered_map<hashed_string, GFX::RenderView> renderViews_;
};

Scene::Scene(std::string_view name, Engine* engine)
{
  data_ = new SceneStorage;
  data_->name_ = name;
  data_->engine_ = engine;
}

Scene::~Scene()
{
  data_->registry_.clear();
  delete data_;
}

Entity Scene::CreateEntity(std::string_view name)
{
  Entity entity(data_->registry_.create(), this);
  auto& tag = entity.AddComponent<Component::Tag>();
  tag.tag = name.empty() ? "Entity" : name;
  return entity;
}

Entity Scene::GetEntity(std::string_view name)
{
  auto view = data_->registry_.view<Component::Tag>();
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
}

void Scene::RegisterRenderView(hashed_string viewName, GFX::RenderView view)
{
  ASSERT(!data_->renderViews_.contains(viewName));
  data_->renderViews_.emplace(viewName, view);
}

void Scene::UnregisterRenderView(hashed_string viewName)
{
  ASSERT(data_->renderViews_.contains(viewName));
  data_->renderViews_.erase(viewName);
}

GFX::RenderView& Scene::GetRenderView(hashed_string viewName)
{
  ASSERT(data_->renderViews_.contains(viewName));
  return data_->renderViews_[viewName];
}

std::vector<std::pair<hashed_string, GFX::RenderView>> Scene::GetRenderViewsWithNames()
{
  std::vector<std::pair<hashed_string, GFX::RenderView>> views(
    data_->renderViews_.begin(), data_->renderViews_.end());
  return views;
}

std::vector<GFX::RenderView> Scene::GetRenderViews()
{
  std::vector<GFX::RenderView> views;
  for (auto& [name, view] : data_->renderViews_)
  {
    views.push_back(view);
  }
  return views;
}

Engine* Scene::GetEngine()
{
  return data_->engine_;
}

std::string_view Scene::GetName()
{
  return data_->name_;
}

entt::registry& Scene::GetRegistry()
{
  return data_->registry_;
}
