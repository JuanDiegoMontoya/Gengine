#pragma once
#include <engine/ecs/ScriptableEntity.h>

class FlyingPlayerController : public ScriptableEntity
{
public:
  void OnCreate() override {}

  void OnDestroy() override {}

  void OnUpdate(Timestep timestep) override;
};