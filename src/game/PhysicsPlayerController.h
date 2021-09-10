#pragma once
#include <engine/ecs/ScriptableEntity.h>

class PhysicsPlayerController : public ScriptableEntity
{
public:
  void OnCreate() override;

  void OnDestroy() override {}

  void OnUpdate(Timestep timestep) override;

  const float jumpVel = 8.0f;

  const float slowSpeed = 2.f;
  const float fastSpeed = 10.f;
  const float normalSpeed = 5.f;
  
  const float slowForce = 60.f;
  const float fastForce = 400.f;
  const float normalForce = 200.f;
};