#pragma once
#include "../../Scene.h"
#include "../../Timestep.h"

class PhysicsSystem
{
public:
  PhysicsSystem();
  ~PhysicsSystem();
  void InitScene(Scene& scene);
  void Update(Scene& scene, Timestep timestep);

private:
};