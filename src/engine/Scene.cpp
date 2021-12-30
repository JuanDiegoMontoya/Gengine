#include "PCH.h"
#include "Scene.h"
#include <entt/entt.hpp>
#include <unordered_map>
#include "ecs/Entity.h"
#include "ecs/component/Core.h"
#include "ecs/component/Transform.h"
#include <engine/gfx/Renderer.h>
#include <engine/gfx/RenderView.h>

// https://www.codeproject.com/Tips/5255442/Cplusplus14-20-Heterogeneous-Lookup-Benchmark
struct MyEqual : public std::equal_to<>
{
  using is_transparent = void;
};

struct string_hash
{
  using is_transparent = void;
  using key_equal = std::equal_to<>;  // Pred to use
  using hash_type = std::hash<std::string_view>;  // just a helper local type
  size_t operator()(std::string_view txt) const { return hash_type{}(txt); }
  size_t operator()(const std::string& txt) const { return hash_type{}(txt); }
  size_t operator()(const char* txt) const { return hash_type{}(txt); }
};

struct SceneStorage
{
  entt::registry registry_{};  // all the entities in this scene
  Engine* engine_{};           // non-owning
  std::string name_;           // the name of this scene
  std::unordered_map<std::string, GFX::RenderView*, string_hash, MyEqual> renderViews_;
};

Scene::Scene(std::string_view name, Engine* engine)
{
  data_ = new SceneStorage;
  data_->name_ = name;
  data_->engine_ = engine;

  data_->renderViews_.emplace("main", GFX::Renderer::GetMainRenderView());
  for (size_t i = 0; i < 6; i++)
  {
    data_->renderViews_.emplace("probe" + std::to_string(i), GFX::Renderer::GetProbeRenderView(i));
  }
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

void Scene::RegisterRenderView(std::string_view viewName, GFX::RenderView* view)
{
  ASSERT(!data_->renderViews_.contains(viewName));
  ASSERT(view->camera);
  data_->renderViews_.emplace(viewName, view);
}

void Scene::UnregisterRenderView(std::string_view viewName)
{
  ASSERT(data_->renderViews_.contains(viewName));
  data_->renderViews_.erase(data_->renderViews_.find(viewName));
}

GFX::RenderView* Scene::GetRenderView(std::string_view viewName)
{
  ASSERT(data_->renderViews_.contains(viewName));
  return data_->renderViews_.find(viewName)->second;
}

std::vector<std::pair<std::string_view, GFX::RenderView*>> Scene::GetRenderViewsWithNames()
{
  std::vector<std::pair<std::string_view, GFX::RenderView*>> views(
    data_->renderViews_.begin(), data_->renderViews_.end());
  return views;
}

std::vector<GFX::RenderView*> Scene::GetRenderViews()
{
  std::vector<GFX::RenderView*> views;
  for (auto& [name, view] : data_->renderViews_)
  {
    if (name == "main")
      views.insert(views.begin(), view);
    else
      views.push_back(view);
  }
  std::reverse(views.begin(), views.end());
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
