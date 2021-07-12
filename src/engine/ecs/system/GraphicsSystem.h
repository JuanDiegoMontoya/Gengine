#pragma once

class Scene;
struct GLFWwindow;

class GraphicsSystem
{
public:
  void Init();
  void Shutdown();

  void StartFrame();
  void DrawOpaque(Scene& scene, float dt);
  void DrawTransparent(Scene& scene, float dt);
  void EndFrame(float dt);
  void SwapBuffers();

  GLFWwindow* GetWindow() { return window; }
private:
  // TODO: replace with window class
  GLFWwindow* window;
};