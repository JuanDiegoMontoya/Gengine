#include "stdafx.h"
#include "Component.h"
#include "World.h"
#include "GameObject.h"

GameObject::GameObject()
{
	for (size_t i = 0; i < size_t(ComponentType::cCount); i++)
	{
		components_[i] = nullptr;
	}
	// this needs to do some stuff with initializing the object id or something
}


GameObject::~GameObject()
{
	for (int i = 0; i < size_t(ComponentType::cCount); i++)
	{
		if (components_[i])
		{
			delete components_[i];
		}
	}
}


GameObjectPtr GameObject::Clone() const
{
	GameObjectPtr newobj = new GameObject();
	for (size_t i = 0; i < size_t(ComponentType::cCount); i++)
	{
		if (components_[i])
			newobj->AddComponent(components_[i]->Clone());
	}

	// set all necessary member vars
	newobj->enabled_ = enabled_;
	newobj->name_ = name_;

	return newobj;
}


void GameObject::AddComponent(Component* component)
{
	if (component)
	{
		component->SetParent(this);
		components_[component->GetTypei()] = component;
	}
}
