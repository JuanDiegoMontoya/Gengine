#pragma once
#include "../../Timestep.h"

class Scene;

class ScriptSystem
{
public:
  void InitScene(Scene& scene);
  void Update(Scene& scene, Timestep timestep);

private:

};