/*HEADER_GOES_HERE*/
#include "../Headers/Engine.h"
#include "../Headers/Containers/Space.h"
#include "../Headers/Containers/Properties.h"

#include "../Headers/Factory.h"

//events
#include "../Headers/Events/UpdateEvent.h"
//#include "../Headers/Events/DrawEvent.h"
//#include "../Headers/Events/InitEvent.h"
//#include "../Headers/Events/RenderEvent.h"
//#include "../Headers/Events/PreRenderEvent.h"

//managers
#define ENGINE_RUNNING
#include "../Headers/Managers/AllManagerHeaders.h"
#undef ENGINE_RUNNING

#include <iostream>

Engine* Engine::engine = nullptr;

Engine::Engine()
{
  //Register with factory:
  Factory::EngineProperties = std::vector<PropertyID>({
    //PROPERTY_ID(Int, engineData)
    });

  //managers.push_back(Graphics::GetGraphics());
  //managers.push_back(InputManager::GetInputManager());
  //managers.push_back(Audio::GetAudio());
  managers.push_back(TraceManager::GetTraceManager());
  managers.push_back(FrameRateController::GetFrameRateController());
  managers.push_back(GraphicsManager::GetGraphicsManager());
  managers.push_back(InputManager::GetInputManager());
}

Engine::~Engine()
{
  for (auto& i : managers)
    delete i;
}

void Engine::Init()
{
  for (auto& i : managers)
    i->Init();

  spaces.push_back(Space::CreateInitialSpace());

  for (auto& i : spaces)
    i->Init();
}

void Engine::Update()
{
  //Update the input manager
  //InputManager::GetInputManager()->Update();

  //Get the dt 
  float dt = FrameRateController::GetFrameRateController()->Update();
  elapsedTime += dt;

  //Push an update event.
  eventManager->AttachEvent(std::move(UpdateEvent::GenerateUpdateEvent(dt, elapsedTime, HANDLE_INSTANTLY)));

  //Push a draw event....

  //Update the event queue
  eventManager->Update(dt);
}

void Engine::End()
{
  for (auto& i : spaces)
    i->End();
  for (auto& i : managers)
    i->End();
}

Space* Engine::AttachSpace(std::unique_ptr<Space>& spaceToAttach, bool initOnAttach)
{
  spaces.push_back(std::move(spaceToAttach));
  if (initOnAttach)
    spaceToAttach->Init();
  return spaceToAttach.get();
}