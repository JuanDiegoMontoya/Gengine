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

protected:
	virtual void OnCreate() {}
	virtual void OnDestroy() {}
	virtual void OnUpdate(float dt) {}

private:
	Entity entity_;
	friend class ScriptSystem;
};