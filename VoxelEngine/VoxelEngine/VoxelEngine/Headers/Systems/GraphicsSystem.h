#pragma once

#include "System.h"
#include "../FactoryID.h"
#include <Events/UpdateEvent.h>

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

  struct GLFWwindow* window;
};