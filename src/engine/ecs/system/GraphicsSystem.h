#pragma once
#include "../../Timestep.h"

class Scene;
struct GLFWwindow;

class GraphicsSystem
{
public:
  void Init();
  void Shutdown();

  void StartFrame();
  void DrawOpaque(Scene& scene);
  void DrawTransparent(Scene& scene);
  void EndFrame(Timestep timestep);
  void SwapBuffers();

  GLFWwindow* const* GetWindow() { return window; }
private:
  // TODO: replace with window class
  GLFWwindow* const* window{};
};