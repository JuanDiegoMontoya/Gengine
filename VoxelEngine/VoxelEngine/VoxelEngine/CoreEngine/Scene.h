#pragma once
#include <entt.hpp>
#include <string>

class Entity;
class Engine;

class Scene
{
public:
  Scene(std::string_view name, Engine& engine);
  ~Scene();

  Entity CreateEntity(std::string_view name = "");

  // Returns an entity whose tag matches the given name.
  // A null entity is returned if no matches are found.
  // If there are multiple matches, the first match is returned.
  Entity GetEntity(std::string_view name);

  Engine& GetEngine() { return engine_; }
  std::string_view GetName() { return name_; }
  entt::registry& GetRegistry() { return registry_; }

  Scene(Scene&&) noexcept = delete;
  Scene(Scene&) = delete;
  Scene& operator=(const Scene&) = delete;
  Scene& operator=(const Scene&&) noexcept = delete;
  bool operator==(const Scene&) const = delete;

private:
  friend class Entity;
  entt::registry registry_{};

  // non-owning
  Engine& engine_;

  std::string name_;
};