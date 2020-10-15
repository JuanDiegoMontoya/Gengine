#pragma once
#include "Scene.h"
#include <memory>

class GraphicsSystem;
class DebugSystem;

class Engine
{
public:
  Engine();
  ~Engine();

  void Run();

private:
  double dt;
  std::vector<Scene> scenes;

  bool running = true;

  std::unique_ptr<GraphicsSystem> graphicsSystem;
  std::unique_ptr<DebugSystem> debugSystem;
};