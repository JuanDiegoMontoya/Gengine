#ifndef COMPONENT_COUNT
#define COMPONENT_COUNT 3
#endif

#ifdef FACTORY_RUNNING

#include "../Headers/Factory.h"
#include "StubComponent.h"
#include "TestingComponent.h"
#include "Transform.h"


void RegisterComponents()
{
Factory::ComponentPropertyMap["StubComponent"] = std::vector<PropertyID>({
  });
Factory::ComponentPropertyMap["TestingComponent"] = std::vector<PropertyID>({
  PropertyID("componentData", TestingComponent::componentData_id, offsetof(TestingComponent, componentData), sizeof(Int)),
  });
Factory::ComponentPropertyMap["Transform"] = std::vector<PropertyID>({
  });
}

inline std::unique_ptr<Component> Component0() { return std::move(StubComponent::RegisterStubComponent()); }
inline std::unique_ptr<Component> Component1() { return std::move(TestingComponent::RegisterTestingComponent()); }
inline std::unique_ptr<Component> Component2() { return std::move(Transform::RegisterTransform()); }


#endif // !FACTORY_RUNNING