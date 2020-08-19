#ifndef EVENT_COUNT
#define EVENT_COUNT 1
#endif

#ifdef FACTORY_RUNNING

#include "../Headers/Factory.h"
#include "StubEvent.h"


void RegisterEvents()
{
Factory::EventPropertyMap["StubEvent"] = std::vector<PropertyID>({
  });
}

inline std::unique_ptr<Event> Event0() { return std::move(StubEvent::RegisterStubEvent()); }


#endif // !FACTORY_RUNNING