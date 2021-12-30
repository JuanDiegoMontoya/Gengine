#pragma once
#include <string_view>
#include <entt/fwd.hpp>
#include <utility/HashedString.h>
#include <vector>

class Entity;
class Engine;

namespace GFX
{
  struct RenderView;
}

class Scene
{
public:
  Scene(std::string_view name, Engine* engine);
  ~Scene();

  // no copy, no move
  Scene(Scene&&) = delete;
  Scene(Scene&) = delete;
  Scene& operator=(const Scene&) = delete;
  Scene& operator=(const Scene&&) = delete;

  Entity CreateEntity(std::string_view name = "");

  // Returns an entity whose tag matches the given name.
  // A null entity is returned if no matches are found.
  // If there are multiple matches, the first match is returned.
  Entity GetEntity(std::string_view name);

  void RegisterRenderView(std::string_view viewName, GFX::RenderView* view);
  void UnregisterRenderView(std::string_view viewName);
  GFX::RenderView* GetRenderView(std::string_view viewName);
  std::vector<std::pair<std::string_view, GFX::RenderView*>> GetRenderViewsWithNames();
  std::vector<GFX::RenderView*> GetRenderViews();

  Engine* GetEngine();
  std::string_view GetName();
  entt::registry& GetRegistry();

private:
  struct SceneStorage* data_; // PIMPL
};