#include "Engine.h"
#include <Utilities/Timer.h>
#include "Input.h"

#include "GraphicsSystem.h"
#include "DebugSystem.h"

// TODO: TEMP GARBAGE
#include <Graphics/Context.h>

Engine::Engine()
{
  graphicsSystem = std::make_unique<GraphicsSystem>();
  debugSystem = std::make_unique<DebugSystem>();

  scenes.push_back(Scene());
}

Engine::~Engine()
{

}

void Engine::Run()
{
  GLFWwindow* window = init_glfw_context();
  debugSystem->Init(window);

  Timer timer;
  while (running)
  {
    dt = timer.elapsed();

    Input::Update();

    for (Scene& scene : scenes)
    {
      debugSystem->StartFrame(scene, dt);

      graphicsSystem->Update(scene, dt);

      debugSystem->EndFrame(scene, dt);
    }
  }
}
