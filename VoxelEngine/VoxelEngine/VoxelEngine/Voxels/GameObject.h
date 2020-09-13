#pragma once
#include "Component.h"


class GameObject
{
public:
	template<typename T>
	inline T* GetComponent(ComponentType typeId)
	{
		return static_cast<T*>(components_[typeId]);
	}

	template<typename T>
	inline T* GetComponent()
	{
		return static_cast<T*>(components_[T::ctype]);
	}

	GameObject();
	~GameObject();
	GameObjectPtr Clone() const;

	void AddComponent(Component* component);
	void SetName(std::string name) { name_ = name; }
	void SetEnabled(bool b) { enabled_ = b; }

	bool GetEnabled() const { return enabled_; }
	Component* GetComponent(unsigned t) { return components_[t]; }
	Component* const* GetAllComponents() { return components_; }
	const std::string& GetName() const { return name_; }

private:
	Component* components_[int(ComponentType::cCount)];
	bool enabled_; // user var
	std::string name_;

};

//using GameObjectPtr = std::shared_ptr<GameObject>;