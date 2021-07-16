#pragma once
#include "Scene.h"
#include "Timestep.h"
#include <memory>

class GraphicsSystem;
class DebugSystem;
class PhysicsSystem;
class ScriptSystem;
class ParticleSystem;
class LifetimeSystem;

class Engine
{
public:
  ~Engine();

  void InitScenes();
  void Run();

  Scene* GetScene(std::string_view name);
  Scene* GetScene(unsigned index);
  void AddScene(Scene* scene);// { scenes.push_back(std::make_unique<Scene>(scene)); }
  void SetActiveScene(std::string_view name);// { activeScene = scene; }
  void SetActiveScene(unsigned index);
  void Pause() { paused_ = true; }
  void Unpause() { paused_ = false; }
  bool IsPaused() const { return paused_; }
  void Stop() { running_ = false; }

  void SetTimescale(float ts) { timescale_ = ts; }
  float GetTimescale() const { return timescale_; }

private:
  friend class Application;
  Engine();

  double timescale_{ 1.0f };
  std::vector<std::unique_ptr<Scene>> scenes_;
  Scene* activeScene_ = nullptr;

  void(*updateCallback)(Timestep) = nullptr;
  void(*drawCallback)(Timestep) = nullptr;

  bool running_ = true;
  bool paused_ = false;

  std::unique_ptr<GraphicsSystem> graphicsSystem;
  std::unique_ptr<DebugSystem> debugSystem;
  std::unique_ptr<PhysicsSystem> physicsSystem;
  std::unique_ptr<ScriptSystem> scriptSystem;
  std::unique_ptr<ParticleSystem> particleSystem;
  std::unique_ptr<LifetimeSystem> lifetimeSystem;
};