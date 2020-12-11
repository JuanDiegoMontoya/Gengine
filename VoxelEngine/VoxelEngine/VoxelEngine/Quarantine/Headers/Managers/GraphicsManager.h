#pragma once

#include "Manager.h"
#include "../FactoryID.h"
#include <Graphics/GlobalRendererInfo.h>

class UpdateEvent;
struct GLFWwindow;
class Camera;

class GraphicsManager : public Manager
{
public:

  static const ID managerType = cGraphicsManager;

  static GraphicsManager* pGraphicsManager;

  static GraphicsManager* const GetGraphicsManager() { SINGLETON(GraphicsManager, pGraphicsManager); }

  ~GraphicsManager();
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

  GLFWwindow* GetWindow()
  {
    return info->window;
  }

private:
  GraphicsManager();

  friend void RegisterManager();

  // RenderContext state; // <- stores ref to camera, possibly other global graphical state (debug?)
  GLFWwindow* window = nullptr;
  std::unique_ptr<GlobalRendererInfo> info;
};

// idk if this sucks or not, just pretend it's a macro
inline Camera* GetCurrentCamera()
{
  return GraphicsManager::GetGraphicsManager()->GetActiveCamera();
}