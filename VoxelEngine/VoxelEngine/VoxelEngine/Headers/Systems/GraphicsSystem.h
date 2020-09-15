#pragma once

#include "System.h"
#include "../FactoryID.h"

class UpdateEvent;

class GraphicsSystem : public System
{
public:

  static const ID systemType = cGraphicsSystem;

  static GraphicsSystem* pGraphicsSystem;

  static GraphicsSystem* const GetGraphicsSystem() { SINGLETON(GraphicsSystem, pGraphicsSystem); }

  ~GraphicsSystem();
  void Init();
  void End();
  std::string GetName();

  void UpdateEventsListen(UpdateEvent* updateEvent);
  void RenderEventsListen(UpdateEvent* updateEvent);

private:
  GraphicsSystem();

  friend void RegisterSystems();

  // RenderContext state; // <- stores ref to camera, possibly other global graphical state (debug?)
  struct GLFWwindow* window = nullptr;
};