#include "PCH.h"
#include <glad/glad.h>
#include "Engine.h"
#include <utility/Timer.h>
#include <engine/Input.h>
#include <engine/GAssert.h>

#include "ecs/system/GraphicsSystem.h"
#include "ecs/system/DebugSystem.h"
#include "ecs/system/PhysicsSystem.h"
#include "ecs/system/ScriptSystem.h"
#include "ecs/system/ParticleSystem.h"
#include "ecs/system/LifetimeSystem.h"

#include "Console.h"
#include "CVar.h"
#include "Parser.h"

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
  while (running_)
  {
    dt_ = static_cast<float>(timer.elapsed()) * timescale_;
    timer.reset();

    if (paused_) dt_ = 0;

    Input::Update();
    debugSystem->StartFrame(*activeScene_, dt_);

    // idk when this should be called tbh
    scriptSystem->Update(*activeScene_, dt_);

    lifetimeSystem->Update(*activeScene_, dt_);

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
