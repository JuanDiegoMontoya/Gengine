#pragma once

#include "System.h"
#include "../FactoryID.h"
#include <Graphics/GlobalRendererInfo.h>

class UpdateEvent;
struct GLFWwindow;
class Camera;

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

  void SetActiveCamera(Camera* camera)
  {
    info->SetCamera(camera);
  }

  Camera* GetActiveCamera()
  {
    return info->activeCamera;
  }

private:
  GraphicsSystem();

  friend void RegisterSystems();

  // RenderContext state; // <- stores ref to camera, possibly other global graphical state (debug?)
  GLFWwindow* window = nullptr;
  std::unique_ptr<GlobalRendererInfo> info;
};

// idk if this sucks or not, just pretend it's a macro
inline Camera* GetCurrentCamera()
{
  GraphicsSystem::GetGraphicsSystem()->GetActiveCamera();
}