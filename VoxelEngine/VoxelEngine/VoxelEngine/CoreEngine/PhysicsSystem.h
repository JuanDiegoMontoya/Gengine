#pragma once
#include <CoreEngine/Scene.h>

class PhysicsSystem
{
public:
  PhysicsSystem();
  ~PhysicsSystem();
  void Update(Scene& scene, float dt);

private:
  const float gravity = .4f;
};