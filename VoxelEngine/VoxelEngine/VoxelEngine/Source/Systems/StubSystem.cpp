/*HEADER_GOES_HERE*/
#include "../../Headers/Systems/StubSystem.h"
#include "../../Headers/Factory.h"

StubSystem* StubSystem::pStubSystem = nullptr;

StubSystem::StubSystem()
{
  //Register with factory:
  Factory::SystemPropertyMap["StubSystem"] = std::vector<PropertyID>({

    });
}

StubSystem::~StubSystem()
{
  pStubSystem = nullptr;
}

std::string StubSystem::GetName()
{
  return "StubSystem";
}

void StubSystem::Init()
{
}

void StubSystem::End()
{
}