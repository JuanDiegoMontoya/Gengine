/*HEADER_GOES_HERE*/
#include "../Headers/Engine.h"
#include "../Headers/Containers/Space.h"
#include "../Headers/Containers/Properties.h"

#include "../Headers/Factory.h"

//events
//#include "../Headers/Events/UpdateEvent.h"
//#include "../Headers/Events/DrawEvent.h"
//#include "../Headers/Events/InitEvent.h"
//#include "../Headers/Events/RenderEvent.h"
//#include "../Headers/Events/PreRenderEvent.h"

//systems
#define ENGINE_RUNNING
#include "../Headers/Systems/AllSystemHeaders.h"
#undef ENGINE_RUNNING

#include <iostream>

Engine* Engine::engine = nullptr;

Engine::Engine()
{
  //Register with factory:
  Factory::EngineProperties = std::vector<PropertyID>({
    //PROPERTY_ID(Int, engineData)
    });

  //systems.push_back(Graphics::GetGraphics());
  //systems.push_back(InputManager::GetInputManager());
  //systems.push_back(Audio::GetAudio());
  //systems.push_back(Trace::GetTrace());
  systems.push_back(FrameRateController::GetFrameRateController());
}

Engine::~Engine()
{
  for (auto& i : systems)
    delete i;
}

void Engine::Init()
{
  for (auto& i : systems)
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
  //eventManager->AttachEvent(std::move(UpdateEvent::GenerateUpdateEvent(dt, elapsedTime, HANDLE_INSTANTLY)));

  //Push a draw event....

  //Update the event queue
  eventManager->Update(dt);
}

void Engine::End()
{
  for (auto& i : spaces)
    i->End();
  for (auto& i : systems)
    i->End();
}

Space* Engine::AttachSpace(std::unique_ptr<Space>& spaceToAttach, bool initOnAttach)
{
  spaces.push_back(std::move(spaceToAttach));
  if (initOnAttach)
    spaceToAttach->Init();
  return &*spaceToAttach;
}