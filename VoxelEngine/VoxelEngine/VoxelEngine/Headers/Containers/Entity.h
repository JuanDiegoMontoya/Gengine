#pragma once
#include <Containers/Space.h>
#include <entt.hpp>
#include <engine_assert.h>

// example system
struct Tag
{
  std::string tag;
};

// "lightweight object"
class Entity
{
public:
  Entity() = default;
  Entity(entt::entity handle, Space* space)
    : entityHandle_(handle), space_(space) {}
  Entity(const Entity& other) = default;

  template<typename T, typename... Args>
  T& AddSystem(Args&&... args)
  {
    ASSERT_MSG(!HasSystem<T>(), "Entity already has system!");
    return space_->registry.emplace<T>(entityHandle_, std::forward<Args>(args)...);
  }

  template<typename T>
  T& GetSystem()
  {
    ASSERT_MSG(HasSystem<T>(), "Entity missing system!");
    return space_->registry.get<T>(entityHandle_);
  }

  template<typename T>
  bool HasSystem()
  {
    return space_->registry.has<T>(entityHandle_);
  }

  template<typename T>
  void RemoveSystem()
  {
    ASSERT_MSG(HasSystem<T>(), "Entity missing system!");
    space_->registry.remove<T>(entityHandle_);
  }

  operator bool() const { return entityHandle_ != entt::null; }
  operator uint32_t() const { return static_cast<uint32_t>(entityHandle_); }

  bool operator==(const Entity& other) const
  {
    return entityHandle_ == other.entityHandle_ && space_ == other.space_;
  }

  bool operator!=(const Entity& other) const
  {
    return !(*this == other);
  }

private:
  entt::entity entityHandle_ = entt::null;
  Space* space_ = nullptr;
};