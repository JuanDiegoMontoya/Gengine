#ifndef AllEventHeaders_Guard
#define AllEventHeaders_Guard

#define EVENT_COUNT 1

#ifdef FACTORY_RUNNING

#include "../Headers/Factory.h"
#include "StubEvent.h"


void RegisterEvents()
{
Factory::EventPropertyMap["StubEvent"] = std::vector<PropertyID>({
  });
}

inline std::unique_ptr<Event> Event0() { return std::make_unique<StubEvent>(StubEvent(); }


#endif // !FACTORY_RUNNING

#endif // !AllEventHeaders_Guard