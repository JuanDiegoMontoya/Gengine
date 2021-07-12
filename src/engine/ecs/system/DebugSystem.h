#pragma once

class Scene;
struct GLFWwindow;

class DebugSystem
{
public:
  void Init(GLFWwindow* win);
  void End();

  void StartFrame(Scene& scene, float dt);
  void Update(Scene& scene, float dt);
  void EndFrame(Scene& scene, float dt);

private:
  GLFWwindow* window;
};