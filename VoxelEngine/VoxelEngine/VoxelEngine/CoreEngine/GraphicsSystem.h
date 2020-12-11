#pragma once

class Scene;
struct GLFWwindow;

class GraphicsSystem
{
public:
  void Init();
  void Shutdown();

  void StartFrame();
  void Update(Scene& scene, float dt);
  void EndFrame();

  GLFWwindow* GetWindow() { return window; }
private:
  // TODO: replace with window class
  GLFWwindow* window;
  unsigned fbo;
  int windowWidth = 1920, windowHeight = 1080;
  int fboWidth = 1920, fboHeight = 1080;
};