#pragma once
#include "../../Timestep.h"

class Scene;
struct GLFWwindow;

class GraphicsSystem
{
public:
  void Init();
  void Shutdown();

  void StartFrame(Scene& scene);
  void DrawOpaque(Scene& scene);
  void DrawSky(Scene& scene);
  void DrawTransparent(Scene& scene);
  void DrawShading(Scene& scene);
  void DrawFog(Scene& scene);
  void DrawEarlyFog(Scene& scene);
  void EndFrame(Scene& scene, Timestep timestep);
  void SwapBuffers();

  GLFWwindow* const* GetWindow() { return window; }
private:
  // TODO: replace with window class
  GLFWwindow* const* window{};
};