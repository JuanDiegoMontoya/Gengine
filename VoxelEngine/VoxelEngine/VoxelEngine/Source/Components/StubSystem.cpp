/*HEADER_GOES_HERE*/
#include "../../Headers/Systems/StubSystem.h"
#include "../../Headers/Containers/Object.h"
#include "../../Headers/Containers/Space.h"
#include "../../Headers/Factory.h"

//#include "../../Headers/Events/UpdateEvent.h"
//#include "../../Headers/Events/DrawEvent.h"

std::string StubSystem::GetName() { return "StubSystem"; }

std::unique_ptr<StubSystem> StubSystem::RegisterStubSystem() 
{
  auto system = std::unique_ptr<StubSystem>(new StubSystem(
    //5999                                            /***Only needed if you don't provide a default constructor***/
  ));

  return std::move(system);
}

StubSystem::StubSystem(
  //Int systemData_
) : System(systemType) //, systemData(systemData_)
{
}

StubSystem::~StubSystem()
{
}

void StubSystem::Init()
{
  //GetSpace()->RegisterListener(this, &StubSystem::UpdateEventsListen);
  //GetSpace()->RegisterListener(this, &StubSystem::DrawEventsListen);
}

void StubSystem::End()
{
  if (parent != nullptr)
  {
  //  GetSpace()->UnregisterListener(this, &StubSystem::UpdateEventsListen);
  //  GetSpace()->UnregisterListener(this, &StubSystem::DrawEventsListen);
  }
}

std::unique_ptr<System> StubSystem::Clone() const
{
  auto result = new StubSystem();
    //copy over values here
  //result->systemData = systemData;

  return std::unique_ptr<System>(result);
}

//void StubSystem::UpdateEventsListen(UpdateEvent* updateEvent) { }

//void StubSystem::DrawEventsListen(DrawEvent* drawEvent) { }