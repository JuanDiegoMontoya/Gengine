#pragma once

#include <entt.hpp>
#include <memory>
#include "Scene.h"

class Entity
{
public:
	Entity(entt::entity handle, Scene* scene)
		: entityHandle_(handle), scene_(scene) {}
	Entity(const Entity& other) = default;

	template<typename T, typename... Args>
	T& AddComponent(Args&&... args)
	{
		assert(!HasComponent<T>() && "Entity already has component!");
		return scene_->registry_.emplace<T>(entityHandle_, std::forward<Args>(args)...);
	}

	template<typename T>
	T& GetComponent()
	{
		assert(HasComponent<T>() && "Entity does not have component!");
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
		scene->registry_.remove<T>(entityHandle_);
	}

private:
	entt::entity entityHandle_{ entt::null };
	Scene* scene_ = nullptr;
};