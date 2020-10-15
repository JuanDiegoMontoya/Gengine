#pragma once

class Scene;
struct GLFWwindow;

class GraphicsSystem
{
public:
  void Init();
  void Shutdown();
  void Update(Scene& scene, float dt);

  GLFWwindow* GetWindow() { return window; }
private:
  // TODO: replace with window class
  GLFWwindow* window;
};