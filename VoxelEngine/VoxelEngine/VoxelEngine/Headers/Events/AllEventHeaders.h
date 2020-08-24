#ifndef EVENT_COUNT
#define EVENT_COUNT 3
#endif

#ifdef FACTORY_RUNNING

#include "../Headers/Factory.h"
#include "DrawEvent.h"
#include "StubEvent.h"
#include "UpdateEvent.h"


void RegisterEvents()
{
Factory::EventPropertyMap["DrawEvent"] = std::vector<PropertyID>({
  });
Factory::EventPropertyMap["StubEvent"] = std::vector<PropertyID>({
  });
Factory::EventPropertyMap["UpdateEvent"] = std::vector<PropertyID>({
  });
}

inline std::unique_ptr<Event> Event0() { return std::move(DrawEvent::RegisterDrawEvent()); }
inline std::unique_ptr<Event> Event1() { return std::move(StubEvent::RegisterStubEvent()); }
inline std::unique_ptr<Event> Event2() { return std::move(UpdateEvent::RegisterUpdateEvent()); }


#endif // !FACTORY_RUNNING