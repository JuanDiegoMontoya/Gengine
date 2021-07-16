#pragma once
#include "../../Timestep.h"

class Scene;

class LifetimeSystem
{
public:
  //LifetimeSystem() {};
  //~LifetimeSystem() {};
  void Update(Scene& scene, Timestep timestep);

private:
};