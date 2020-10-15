#pragma once
#include "Scene.h"
#include <memory>

class GraphicsSystem;
class DebugSystem;
class VoxelSystem;

class Engine
{
public:
  Engine();
  ~Engine();

  void Run();

  void Stop() { running = false; }

private:
  double dt;
  std::vector<Scene> scenes;

  bool running = true;

  std::unique_ptr<GraphicsSystem> graphicsSystem;
  std::unique_ptr<DebugSystem> debugSystem;
  std::unique_ptr<VoxelSystem> voxelSystem;
};