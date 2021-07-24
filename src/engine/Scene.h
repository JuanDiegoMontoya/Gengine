#pragma once
#include <entt/entt.hpp>
#include <string>
#include <string_view>
#include <optional>


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
  std::optional<Entity> GetEntity(std::string_view name);

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

  Engine& engine_;

  std::string name_;
};