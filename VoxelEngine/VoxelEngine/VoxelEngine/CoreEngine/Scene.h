#pragma once
#include <entt.hpp>

class Entity;

class Scene
{
public:
  Entity CreateEntity(std::string_view name = "");
  void RemoveEntity(std::string_view name);
  void RemoveEntity(Entity entity);

private:
  friend class Entity;
  entt::registry registry_;
};