#include "GraphicsSystem.h"
#include "Scene.h"
#include <Graphics/GraphicsIncludes.h>
#include "Input.h"
#include "Engine.h"
#include "Camera.h"

// TODO: TEMP GARBAGE
#include <Graphics/Context.h>

Camera* cam;

void GraphicsSystem::Init()
{
  cam = new Camera();
  Camera::ActiveCamera = cam;
  window = init_glfw_context();
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void GraphicsSystem::Shutdown()
{
  delete cam;
  glfwDestroyWindow(window);
}

void GraphicsSystem::Update(Scene& scene, float dt)
{
  // TODO: bad bad bad
  if (Input::IsKeyDown(GLFW_KEY_ESCAPE))
    scene.GetEngine().Stop();
  Camera::ActiveCamera->Update(dt);

  //glClearColor(0, 1, 1, 1);
  //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glfwSwapBuffers(window);
}
