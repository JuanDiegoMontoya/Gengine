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
  void RemoveEntity(std::string_view name);
  void RemoveEntity(Entity entity);

  Engine& GetEngine() { return engine_; }
  std::string_view GetName() { return name_; }
  entt::registry& GetRegistry() { return registry_; }

  Scene(Scene&&) = delete;
  Scene(Scene&) = delete;
  Scene& operator=(const Scene&) = delete;
  Scene& operator=(const Scene&&) = delete;
  bool operator==(const Scene&) const = delete;

private:
  friend class Entity;
  entt::registry registry_{};

  // non-owning
  Engine& engine_;

  std::string name_;
};