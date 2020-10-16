#pragma once
#include "Scene.h"
#include <memory>

class GraphicsSystem;
class DebugSystem;
class VoxelSystem;
class PhysicsSystem;

class Engine
{
public:
  Engine();
  ~Engine();

  void Run();

  Scene* GetScene(std::string_view name);
  Scene* GetScene(unsigned index);
  void AddScene(Scene* scene);// { scenes.push_back(std::make_unique<Scene>(scene)); }
  void SetActiveScene(std::string_view name);// { activeScene = scene; }
  void SetActiveScene(unsigned index);

private:
  friend class Application;
  void Stop() { running_ = false; }

  double dt_{};
  std::vector<std::unique_ptr<Scene>> scenes_;
  Scene* activeScene_ = nullptr;

  bool running_ = true;

  std::unique_ptr<GraphicsSystem> graphicsSystem;
  std::unique_ptr<DebugSystem> debugSystem;
  std::unique_ptr<VoxelSystem> voxelSystem;
  std::unique_ptr<PhysicsSystem> physicsSystem;
};