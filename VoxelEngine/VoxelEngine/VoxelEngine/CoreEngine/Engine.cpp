#include "EnginePCH.h"
#include "Engine.h"
#include <Utilities/Timer.h>
#include <CoreEngine/Input.h>
#include <CoreEngine/GAssert.h>

#include "GraphicsSystem.h"
#include "DebugSystem.h"
#include "PhysicsSystem.h"
#include "ScriptSystem.h"
#include "ParticleSystem.h"

#include <CoreEngine/Components.h>

Engine::Engine()
{
  graphicsSystem = std::make_unique<GraphicsSystem>();
  debugSystem = std::make_unique<DebugSystem>();
  physicsSystem = std::make_unique<PhysicsSystem>();
  scriptSystem = std::make_unique<ScriptSystem>();
  particleSystem = std::make_unique<ParticleSystem>();

  graphicsSystem->Init();
  debugSystem->Init(graphicsSystem->GetWindow());
}

Engine::~Engine()
{
  scenes_.~vector();
  physicsSystem.reset();
}

void Engine::Run()
{
  Timer timer;
  while (running_)
  {
    dt_ = static_cast<float>(timer.elapsed());
    timer.reset();

    if (paused_) dt_ = 0;

    Input::Update();
    debugSystem->StartFrame(*activeScene_, dt_);

    // idk when this should be called tbh
    scriptSystem->Update(*activeScene_, dt_);

    if (updateCallback != nullptr)
    {
      updateCallback(dt_);
    }

    particleSystem->Update(*activeScene_, dt_);

    graphicsSystem->StartFrame();
    graphicsSystem->DrawOpaque(*activeScene_, dt_);
    if (drawCallback)
    {
      drawCallback(dt_);
    }
    graphicsSystem->DrawTransparent(*activeScene_, dt_);

    physicsSystem->Update(*activeScene_, dt_);
    debugSystem->Update(*activeScene_, dt_);

    graphicsSystem->EndFrame(dt_);
    debugSystem->EndFrame(*activeScene_, dt_); // render UI on top of everything else
    graphicsSystem->SwapBuffers();
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
