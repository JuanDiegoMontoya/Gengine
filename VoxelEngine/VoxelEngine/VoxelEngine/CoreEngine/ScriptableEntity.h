#pragma once
#include "Entity.h"

class ScriptableEntity
{
public:
	virtual ~ScriptableEntity() {}

	template<typename T>
	T& GetComponent()
	{
		return entity_.GetComponent<T>();
	}

	template<typename T>
	bool HasComponent() const
	{
		return entity_.HasComponent<T>();
	}

	Entity Self()
	{
		return entity_;
	}

	Scene* GetScene()
	{
		return entity_.scene_;
	}

	Entity CreateEntity(std::string_view name = "")
	{
		return entity_.scene_->CreateEntity(name);
	}

protected:
	virtual void OnCreate() {}
	virtual void OnDestroy() {}
	virtual void OnUpdate([[maybe_unused]] float dt) {}

private:
	Entity entity_;
	friend class ScriptSystem;
};