#pragma once
#include <engine/ecs/ScriptableEntity.h>
#include <engine/ecs/component/Physics.h>

class KinematicPlayerController : public ScriptableEntity
{
public:
  void OnCreate() override
  {
  }

  void OnDestroy() override
  {

  }

  void OnUpdate(Timestep timestep) override;

  // instantly sets Y velocity to this when jumping
  const float jumpVel = 8.0f;

  // amount of velocity to lose per second from gravity
  const float gravity = -15.f;

  // amount of velocity to gain/lose per second when moving/not moving
  const float accelerationGround = 60.0f;
  const float accelerationGroundSlow = 30.0f;
  const float accelerationAir = 20.0f;
  const float decelerationGround = 60.0f;
  const float decelerationAir = 3.0f;

  // max speed on the XZ plane when moving
  const float slowSpeed = 2.0f;
  const float normalSpeed = 4.0f;
  const float fastSpeed = 6.0f;

  // displacement after one second
  glm::vec3 velocity{ 0, 0, 0 };

  // fix step bookkeeping
  const double tick = 1.0f / 200.0f;
  float accumulator = 0;
  Physics::ControllerCollisionFlags flags{};

  // misc
  bool activeCursor{ false };
};