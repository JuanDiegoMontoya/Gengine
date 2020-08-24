#ifndef COMPONENT_COUNT
#define COMPONENT_COUNT 2
#endif

#ifdef FACTORY_RUNNING

#include "../Headers/Factory.h"
#include "StubComponent.h"
#include "TestingComponent.h"


void RegisterComponents()
{
Factory::ComponentPropertyMap["StubComponent"] = std::vector<PropertyID>({
  });
Factory::ComponentPropertyMap["TestingComponent"] = std::vector<PropertyID>({
  });
}

inline std::unique_ptr<Component> Component0() { return std::move(StubComponent::RegisterStubComponent()); }
inline std::unique_ptr<Component> Component1() { return std::move(TestingComponent::RegisterTestingComponent()); }


#endif // !FACTORY_RUNNING