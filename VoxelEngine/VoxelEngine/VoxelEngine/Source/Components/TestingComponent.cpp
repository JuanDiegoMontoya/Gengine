/*HEADER_GOES_HERE*/
#include "../../Headers/Components/TestingComponent.h"
#include "../../Headers/Containers/Object.h"
#include "../../Headers/Containers/Space.h"
#include "../../Headers/Factory.h"
#include "../../Headers/PreProcessorMagic.h"

#include "../../Headers/Systems/FrameRateController.h"

#include "../../Headers/Events/UpdateEvent.h"
#include "../../Headers/Events/DrawEvent.h"
#include "../../Headers/Events/TraceEvent.h"
#include "../../Headers/Events/TestingEvent.h"

#include "../../Headers/Containers/BidirectionalMap.h"

std::string TestingComponent::GetName() { return "TestingComponent"; }

std::unique_ptr<TestingComponent> TestingComponent::RegisterTestingComponent() 
{
  auto component = std::unique_ptr<TestingComponent>(new TestingComponent(
    5999                                            /***Only needed if you don't provide a default constructor***/
  ));

  return std::move(component);
}

TestingComponent::TestingComponent(
  Int componentData_
) : Component(componentType) //, componentData(componentData_)
{
}

TestingComponent::~TestingComponent()
{
}

void TestingComponent::Init()
{
  GetSpace()->RegisterListener(this, &TestingComponent::UpdateEventsListen);
  GetSpace()->RegisterListener(this, &TestingComponent::DrawEventsListen);
  parent->RegisterListener(this, &TestingComponent::TestingEventsListen);
}

void TestingComponent::End()
{
  GetSpace()->UnregisterListener(this, &TestingComponent::UpdateEventsListen);
  GetSpace()->UnregisterListener(this, &TestingComponent::DrawEventsListen);
  if (parent != nullptr)
  {
    parent->UnregisterListener(this, &TestingComponent::TestingEventsListen);
  }
}

std::unique_ptr<Component> TestingComponent::Clone() const
{
  auto result = new TestingComponent();
    //copy over values here
  result->componentData = componentData;

  return std::unique_ptr<Component>(result);
}

void TestingComponent::UpdateEventsListen(UpdateEvent* updateEvent) 
{ 
  DoOnce
  {
    WRITE(std::to_string(componentData));

    //Property System
    System* foo = FrameRateController::GetFrameRateController();
    Factory::GetPropertyValue<System, Bool>(FrameRateController::GetFrameRateController(), "locked") = false;


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

void TestingComponent::DrawEventsListen(DrawEvent* drawEvent) { }


void TestingComponent::TestingEventsListen(TestingEvent* testingEvent)
{
  static int counter = 0;

  WRITE(std::string("this is testing event # ") + std::to_string(counter++));
}