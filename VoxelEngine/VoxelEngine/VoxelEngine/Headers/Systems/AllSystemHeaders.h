#ifndef SYSTEM_COUNT
#define SYSTEM_COUNT 4
#endif

#ifdef FACTORY_RUNNING

#include "../Headers/Factory.h"
#include "Camera.h"
#include "StubSystem.h"
#include "TestingSystem.h"
#include "VoxelWorld.h"


void RegisterSystems()
{
Factory::SystemPropertyMap["Camera"] = std::vector<PropertyID>({
  });
Factory::SystemPropertyMap["StubSystem"] = std::vector<PropertyID>({
  });
Factory::SystemPropertyMap["TestingSystem"] = std::vector<PropertyID>({
  PropertyID("systemData", TestingSystem::systemData_id, offsetof(TestingSystem, systemData), sizeof(Int)),
  });
Factory::SystemPropertyMap["VoxelWorld"] = std::vector<PropertyID>({
  });
}

inline std::unique_ptr<System> System0() { return std::move(Camera::RegisterCamera()); }
inline std::unique_ptr<System> System1() { return std::move(StubSystem::RegisterStubSystem()); }
inline std::unique_ptr<System> System2() { return std::move(TestingSystem::RegisterTestingSystem()); }
inline std::unique_ptr<System> System3() { return std::move(VoxelWorld::RegisterVoxelWorld()); }


#endif // !FACTORY_RUNNING