#include "Engine.h"
#include <Utilities/Timer.h>
#include "Input.h"
#include <GAssert.h>

#include "GraphicsSystem.h"
#include "DebugSystem.h"
#include "VoxelSystem.h"


Engine::Engine()
{
  graphicsSystem = std::make_unique<GraphicsSystem>();
  debugSystem = std::make_unique<DebugSystem>();
  voxelSystem = std::make_unique<VoxelSystem>();
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
  while (running_)
  {
    dt_ = timer.elapsed();
    timer.reset();

    Input::Update();

    graphicsSystem->StartFrame();
    debugSystem->StartFrame(*activeScene_, dt_);

    voxelSystem->Draw();
    graphicsSystem->Update(*activeScene_, dt_);
    debugSystem->Update(*activeScene_, dt_);

    debugSystem->EndFrame(*activeScene_, dt_);
    graphicsSystem->EndFrame();
  }

  graphicsSystem->Shutdown();
}

Scene* Engine::GetScene(std::string_view name)
{
  auto fr = std::find_if(scenes_.begin(), scenes_.end(), [name](const auto& ptr) { return name == ptr->GetName(); });
  ASSERT(fr != scenes_.end());
  return fr->get();
}

Scene* Engine::GetScene(unsigned index)
{
  ASSERT(index < scenes_.size());
  return scenes_[index].get();
}

void Engine::AddScene(Scene* scene)
{
  scenes_.emplace_back(scene);
}

void Engine::SetActiveScene(std::string_view name)
{
  auto fr = std::find_if(scenes_.begin(), scenes_.end(), [name](const auto& ptr) { return name == ptr->GetName(); });
  ASSERT(fr != scenes_.end());
  activeScene_ = fr->get();
}

void Engine::SetActiveScene(unsigned index)
{
  ASSERT(index < scenes_.size());
  activeScene_ = scenes_[index].get();
}
