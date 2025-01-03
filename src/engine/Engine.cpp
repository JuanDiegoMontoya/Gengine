#include "PCH.h"
#include <glad/glad.h>
#include "Engine.h"
#include <utility/Timer.h>
#include <engine/Input.h>
#include <engine/GAssert.h>
#include "core/StatMacros.h"

#include "ecs/system/GraphicsSystem.h"
#include "ecs/system/DebugSystem.h"
#include "ecs/system/PhysicsSystem.h"
#include "ecs/system/ScriptSystem.h"
#include "ecs/system/ParticleSystem.h"
#include "ecs/system/LifetimeSystem.h"

#include "Console.h"
#include "CVar.h"
#include "Parser.h"

DECLARE_FLOAT_STAT(MainLoop, CPU)

Engine::Engine()
{
  graphicsSystem = std::make_unique<GraphicsSystem>();
  debugSystem = std::make_unique<DebugSystem>();
  physicsSystem = std::make_unique<PhysicsSystem>();
  scriptSystem = std::make_unique<ScriptSystem>();
  particleSystem = std::make_unique<ParticleSystem>();
  lifetimeSystem = std::make_unique<LifetimeSystem>();

  graphicsSystem->Init();
  debugSystem->Init(graphicsSystem->GetWindow());
  Input::init_glfw_input_cbs(graphicsSystem->GetWindow());

  auto exitFunc = [this](const char*)
  {
    this->Stop();
  };

  auto helpFunc = [](const char* arg)
  {
    CmdParser parser(arg);
    std::vector<CmdAtom> atoms;
    Identifier* identifier = nullptr;
    if (arg[0] != 0)
    {
      while (parser.Valid())
      {
        atoms.push_back(parser.NextAtom());
      }
      identifier = std::get_if<Identifier>(&atoms[0]);
    }


    if (const char* desc;
      atoms.empty() ||
      !identifier ||
      (desc = Console::Get()->GetCommandDesc(identifier->name.c_str())) == nullptr)
    {
      Console::Get()->Log("Usage: help <convar>");
    }
    else
    {
      Console::Get()->Log("%s\n%s", identifier->name.c_str(), desc);
    }
  };

  Console::Get()->RegisterCommand("exit", "- Exits the engine", exitFunc);
  Console::Get()->RegisterCommand("quit", "- Exits the engine", exitFunc);
  Console::Get()->RegisterCommand("help", "- Find help about a convar / concommand", helpFunc);
  Console::Get()->RegisterCommand("clear", "- Clears the console", [](const char*) { Console::Get()->Clear(); });
  CVarSystem::Get()->RegisterCVar("timescale", "- Engine timescale", 1.0, .0001, 1000, CVarFlag::CHEAT, [this](const char*, cvar_float ts)
    {
      this->SetTimescale(ts);
    });
}

Engine::~Engine()
{
  scenes_.~vector();
  physicsSystem.reset();
}

void Engine::InitScenes()
{
  for (auto& scene : scenes_)
  {
    physicsSystem->InitScene(*scene);
    particleSystem->InitScene(*scene);
    scriptSystem->InitScene(*scene);
  }
}

void Engine::Run()
{
  Timer timer;
  Timestep timestep;
  while (running_)
  {
    MEASURE_CPU_TIMER_STAT(MainLoop);

    double dt = static_cast<float>(timer.Elapsed());
    timer.Reset();

    timestep.dt_actual = dt;
    timestep.dt_effective = dt * timescale_;

    if (paused_) timestep.dt_effective = 0;

    Input::Update();
    debugSystem->StartFrame(*activeScene_);

    // idk when this should be called tbh
    scriptSystem->Update(*activeScene_, timestep);

    lifetimeSystem->Update(*activeScene_, timestep);

    if (updateCallback != nullptr)
    {
      updateCallback(timestep);
    }

    particleSystem->Update(*activeScene_, timestep);

    physicsSystem->Update(*activeScene_, timestep);
    debugSystem->Update(*activeScene_, timestep);

    graphicsSystem->StartFrame(*activeScene_);
    graphicsSystem->DrawOpaque(*activeScene_);
    if (drawOpaqueCallback)
    {
      drawOpaqueCallback(activeScene_, timestep);
    }
    graphicsSystem->DrawSky(*activeScene_);
    graphicsSystem->DrawEarlyFog(*activeScene_);
    graphicsSystem->DrawShading(*activeScene_);
    graphicsSystem->DrawFog(*activeScene_);
    graphicsSystem->DrawTransparent(*activeScene_);
    if (drawInterfacePrePostProcessingCallback)
    {
      drawInterfacePrePostProcessingCallback(activeScene_, timestep);
    }
    graphicsSystem->Bloom();
    if (drawInterfacePostPostProcessingCallback)
    {
      drawInterfacePostPostProcessingCallback(activeScene_, timestep);
    }
    graphicsSystem->EndFrame(timestep);
    debugSystem->EndFrame(*activeScene_); // render UI on top of everything else
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
