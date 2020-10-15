#pragma once
#include "Scene.h"
#include <entt.hpp>
#include <GAssert.h>

// "lightweight object"
class Entity
{
public:
  Entity() = default;
  Entity(entt::entity handle, Scene* scene)
    : entityHandle_(handle), scene_(scene) {}
  Entity(const Entity& other) = default;

  template<typename T, typename... Args>
  T& AddComponent(Args&&... args)
  {
    ASSERT_MSG(!HasComponent<T>(), "Entity already has system!");
    return scene_->registry_.emplace<T>(entityHandle_, std::forward<Args>(args)...);
  }

  template<typename T>
  T& GetComponent()
  {
    ASSERT_MSG(HasComponent<T>(), "Entity missing system!");
    return scene_->registry_.get<T>(entityHandle_);
  }

  template<typename T>
  bool HasComponent()
  {
    return scene_->registry_.has<T>(entityHandle_);
  }

  template<typename T>
  void RemoveComponent()
  {
    ASSERT_MSG(HasComponent<T>(), "Entity missing system!");
    scene_->registry_.remove<T>(entityHandle_);
  }

  operator bool() const { return entityHandle_ != entt::null; }
  operator uint32_t() const { return static_cast<uint32_t>(entityHandle_); }

  bool operator==(const Entity& other) const
  {
    return entityHandle_ == other.entityHandle_ && scene_ == other.scene_;
  }

  bool operator!=(const Entity& other) const
  {
    return !(*this == other);
  }

private:
  entt::entity entityHandle_ = entt::null;
  Scene* scene_ = nullptr;
};