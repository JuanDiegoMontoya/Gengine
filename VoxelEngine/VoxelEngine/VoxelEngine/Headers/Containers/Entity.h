#pragma once
#include <Containers/Space.h>
#include <entt.hpp>

// "lightweight object"
class Entity
{
public:
	Entity() = default;
	Entity(entt::entity handle, Space* space)
		: entityHandle_(handle), space_(space) {}
	Entity(const Entity& other) = default;

	template<typename T, typename... Args>
	T& AddComponent(Args&&... args)
	{
		ASSERT_MSG(!HasComponent<T>(), "Entity already has component!");
		return space_->registry.emplace<T>(entityHandle_, std::forward<Args>(args)...);
	}

	template<typename T>
	T& GetComponent()
	{
		ASSERT_MSG(HasComponent<T>(), "Entity missing component!");
		return m_Scene->m_Registry.get<T>(entityHandle_);
	}

	template<typename T>
	bool HasComponent()
	{
		return space_->registry.has<T>(entityHandle_);
	}

	template<typename T>
	void RemoveComponent()
	{
		ASSERT_MSG(HasComponent<T>(), "Entity missing component!");
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