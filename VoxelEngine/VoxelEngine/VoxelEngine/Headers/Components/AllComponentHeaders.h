#ifndef COMPONENT_COUNT
#define COMPONENT_COUNT 5
#endif

#ifdef FACTORY_RUNNING

#include "../Headers/Factory.h"
#include "Camera.h"
#include "StubComponent.h"
#include "Tag.h"
#include "TestingComponent.h"
#include "VoxelWorld.h"


void RegisterComponents()
{
Factory::ComponentPropertyMap["Camera"] = std::vector<PropertyID>({
  });
Factory::ComponentPropertyMap["StubComponent"] = std::vector<PropertyID>({
  });
Factory::ComponentPropertyMap["Tag"] = std::vector<PropertyID>({
  });
Factory::ComponentPropertyMap["TestingComponent"] = std::vector<PropertyID>({
  PropertyID("componentData", TestingComponent::componentData_id, offsetof(TestingComponent, componentData), sizeof(Int)),
  });
Factory::ComponentPropertyMap["VoxelWorld"] = std::vector<PropertyID>({
  });
}

inline std::unique_ptr<Component> Component0() { return std::move(Camera::RegisterCamera()); }
inline std::unique_ptr<Component> Component1() { return std::move(StubComponent::RegisterStubComponent()); }
inline std::unique_ptr<Component> Component2() { return std::move(Tag::RegisterTag()); }
inline std::unique_ptr<Component> Component3() { return std::move(TestingComponent::RegisterTestingComponent()); }
inline std::unique_ptr<Component> Component4() { return std::move(VoxelWorld::RegisterVoxelWorld()); }


#endif // !FACTORY_RUNNING