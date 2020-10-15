#pragma once
#include <entt.hpp>

class Entity;
class Engine;

class Scene
{
public:
  Scene(Engine& engine) : engine_(engine) {}
  Entity CreateEntity(std::string_view name = "");
  void RemoveEntity(std::string_view name);
  void RemoveEntity(Entity entity);

  Engine& GetEngine() { return engine_; }

private:
  friend class Entity;
  entt::registry registry_{};

  // non-owning
  Engine& engine_;
};