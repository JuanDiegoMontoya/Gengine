#ifndef COMPONENT_COUNT
#define COMPONENT_COUNT 1
#endif

#ifdef FACTORY_RUNNING

#include "../Headers/Factory.h"
#include "StubComponent.h"


void RegisterComponents()
{
Factory::ComponentPropertyMap["StubComponent"] = std::vector<PropertyID>({
  });
}

inline std::unique_ptr<Component> Component0() { return std::move(StubComponent::RegisterStubComponent()); }


#endif // !FACTORY_RUNNING