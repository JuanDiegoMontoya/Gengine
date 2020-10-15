/*HEADER_GOES_HERE*/
#include "../../Headers/Systems/TestingSystem.h"
#include "../../Headers/Containers/Object.h"
#include "../../Headers/Containers/Space.h"
#include "../../Headers/Factory.h"
#include "../../Headers/PreProcessorMagic.h"

#include "../../Headers/Managers/FrameRateController.h"

#include "../../Headers/Events/UpdateEvent.h"
#include "../../Headers/Events/DrawEvent.h"
#include "../../Headers/Events/TraceEvent.h"
#include "../../Headers/Events/TestingEvent.h"

#include "../../Headers/Containers/BidirectionalMap.h"

std::string TestingSystem::GetName() { return "TestingSystem"; }

std::unique_ptr<TestingSystem> TestingSystem::RegisterTestingSystem() 
{
  auto system = std::unique_ptr<TestingSystem>(new TestingSystem(
    5999                                            /***Only needed if you don't provide a default constructor***/
  ));

  return std::move(system);
}

TestingSystem::TestingSystem(
  Int systemData_
) : System(systemType) //, systemData(systemData_)
{
}

TestingSystem::~TestingSystem()
{
}

void TestingSystem::Init()
{
  GetSpace()->RegisterListener(this, &TestingSystem::UpdateEventsListen);
  GetSpace()->RegisterListener(this, &TestingSystem::DrawEventsListen);
  parent->RegisterListener(this, &TestingSystem::TestingEventsListen);
}

void TestingSystem::End()
{
  GetSpace()->UnregisterListener(this, &TestingSystem::UpdateEventsListen);
  GetSpace()->UnregisterListener(this, &TestingSystem::DrawEventsListen);
  if (parent != nullptr)
  {
    parent->UnregisterListener(this, &TestingSystem::TestingEventsListen);
  }
}

std::unique_ptr<System> TestingSystem::Clone() const
{
  auto result = new TestingSystem();
    //copy over values here
  result->systemData = systemData;

  return std::unique_ptr<System>(result);
}

void TestingSystem::UpdateEventsListen(UpdateEvent* updateEvent) 
{ 
  DoOnce
  {
    WRITE(std::to_string(systemData));

    //Property System
    Manager* foo = FrameRateController::GetFrameRateController();
    Factory::GetPropertyValue<Manager, Bool>(FrameRateController::GetFrameRateController(), "locked") = false;


    //Event system
    for (float i = 5.f; i < 20.f; i += 5.f)
    {
      auto testingEvent = CLONE_EVENT(TestingEvent);
      testingEvent->timer = i;
      parent->AttachEvent(std::move(testingEvent));
    }

    //File IO 
    uint8_t data = '*';
    Factory::WriteFile("test", &data, 1);
    const std::shared_ptr<Memory> result = Factory::ReadFile("test");
    data = 0;
    data = *result->data;

    //Bimap test
    static bimap<const ID, const std::string> bimap;
    bimap.insert({ 420, "bi" });
    bimap.insert({ 69, "map" });

    if (bimap.has("bi"))
    {
      WRITE(std::string("bi is in the bimap"));
    }
    
  }
  //WRITE(std::to_string(updateEvent->elapsedTime));
}

void TestingSystem::DrawEventsListen(DrawEvent* drawEvent) { }


void TestingSystem::TestingEventsListen(TestingEvent* testingEvent)
{
  static int counter = 0;

  WRITE(std::string("this is testing event # ") + std::to_string(counter++));
}