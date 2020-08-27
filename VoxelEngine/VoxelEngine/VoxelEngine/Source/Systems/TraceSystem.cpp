/*HEADER_GOES_HERE*/
#include "../../Headers/Systems/TraceSystem.h"
#include "../../Headers/Factory.h"
#include "../../Headers/Engine.h"

#include "../../Headers/Events/TraceEvent.h"

TraceSystem* TraceSystem::pTraceSystem = nullptr;

TraceSystem::TraceSystem()
{
}

TraceSystem::~TraceSystem()
{
  pTraceSystem = nullptr;
}

std::string TraceSystem::GetName()
{
  return "TraceSystem";
}

void TraceSystem::Init()
{
  Engine::GetEngine()->RegisterListener(this, &TraceSystem::TraceEventsListen);
}

void TraceSystem::End()
{
  Engine::GetEngine()->UnregisterListener(this, &TraceSystem::TraceEventsListen);
}

void TraceSystem::TraceEventsListen(TraceEvent* traceEvent)
{
  std::cout << traceEvent->traceMessage << '\n';
}