#pragma once
//#include "game_object.h"

typedef class GameObject* GameObjectPtr;

// defines all of the component types, and in which order they are updated
enum class ComponentType : unsigned
{
	cTransform,
	cAABBCollider, // requires physics component
	cLight,
	cMesh,
	cPhysics,
	cText,
	cScripts,
	cRenderData,

	cCount
};


// augments game objects
class Component
{
public:
	void SetType(ComponentType t) { type_ = t; }
	void SetParent(GameObjectPtr p) { parent_ = p; }
	void SetEnabled(bool e) { enabled_ = e; }

	ComponentType GetType() const { return type_; }
	int GetTypei() const { return int(type_); }
	GameObjectPtr GetParent() const { return parent_; }
	bool GetEnabled() const { return enabled_; }

	virtual void Update(float dt) {}
	virtual ~Component() {}
	virtual Component* Clone() const { return nullptr; };

private:
	GameObjectPtr parent_;	// what object it's attached to
	ComponentType type_;		// what type of component
	bool enabled_ = true;		// user var
};