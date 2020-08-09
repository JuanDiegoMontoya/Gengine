#pragma once
#include "component.h"

// requires object to have transform
class PhysicsComp : public Component
{
public:
	PhysicsComp();
	~PhysicsComp() override;
	PhysicsComp* Clone() const override;
	void Update(float dt) override;
private:
	float mass_;
	glm::vec3 oldPosition_;
	glm::vec3 velocity_;
	glm::vec3 acceleration_;
	float gravityScale_; // modifies accel due to gravity
};