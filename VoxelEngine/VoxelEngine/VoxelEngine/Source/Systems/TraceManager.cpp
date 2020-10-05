/*HEADER_GOES_HERE*/
#include "../../Headers/Managers/TraceManager.h"
#include "../../Headers/Factory.h"
#include "../../Headers/Engine.h"

#include "../../Headers/Events/TraceEvent.h"

TraceManager* TraceManager::pTraceManager = nullptr;

TraceManager::TraceManager()
{
}

TraceManager::~TraceManager()
{
  pTraceManager = nullptr;
}

std::string TraceManager::GetName()
{
  return "TraceManager";
}

void TraceManager::Init()
{
  Engine::GetEngine()->RegisterListener(this, &TraceManager::TraceEventsListen);
}

void TraceManager::End()
{
  Engine::GetEngine()->UnregisterListener(this, &TraceManager::TraceEventsListen);
}

void TraceManager::TraceEventsListen(TraceEvent* traceEvent)
{
  std::cout << traceEvent->traceMessage << '\n';
}