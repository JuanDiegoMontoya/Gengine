#pragma once
#include "../../Timestep.h"

class Scene;
struct GLFWwindow;

class DebugSystem
{
public:
  void Init(GLFWwindow* win);
  void End();

  void StartFrame(Scene& scene);
  void Update(Scene& scene, Timestep timestep);
  void EndFrame(Scene& scene);

private:
  GLFWwindow* window;
};