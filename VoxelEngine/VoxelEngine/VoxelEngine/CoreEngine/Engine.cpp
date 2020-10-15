#include "Engine.h"
#include <Utilities/Timer.h>
#include "Input.h"

#include "GraphicsSystem.h"
#include "DebugSystem.h"
#include "VoxelSystem.h"


Engine::Engine()
{
  graphicsSystem = std::make_unique<GraphicsSystem>();
  debugSystem = std::make_unique<DebugSystem>();
  voxelSystem = std::make_unique<VoxelSystem>();

  scenes.push_back(Scene(*this));
}

Engine::~Engine()
{
}

void Engine::Run()
{
  graphicsSystem->Init();
  debugSystem->Init(graphicsSystem->GetWindow());
  voxelSystem->Init();

  Timer timer;
  while (running)
  {
    dt = timer.elapsed();
    timer.reset();

    Input::Update();

    graphicsSystem->StartFrame();
    for (Scene& scene : scenes)
    {
      debugSystem->StartFrame(scene, dt);

      voxelSystem->Draw();
      graphicsSystem->Update(scene, dt);
      debugSystem->Update(scene, dt);

      debugSystem->EndFrame(scene, dt);
    }
    graphicsSystem->EndFrame();
  }

  graphicsSystem->Shutdown();
}
