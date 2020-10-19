#pragma once
#include <CoreEngine/Scene.h>

class PhysicsSystem
{
public:
  PhysicsSystem();
  ~PhysicsSystem();
  void Update(Scene& scene, float dt);

private:
  const float gravity = 9.8f;
};