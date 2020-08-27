/*HEADER_GOES_HERE*/
#include "../../Headers/Components/TestingComponent.h"
#include "../../Headers/Containers/Object.h"
#include "../../Headers/Containers/Space.h"
#include "../../Headers/Factory.h"
#include "../../Headers/PreProcessorMagic.h"

#include "../../Headers/Events/UpdateEvent.h"
#include "../../Headers/Events/DrawEvent.h"
#include "../../Headers/Events/TraceEvent.h"

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
}

void TestingComponent::End()
{
  if (parent != nullptr)
  {
    GetSpace()->UnregisterListener(this, &TestingComponent::UpdateEventsListen);
    GetSpace()->UnregisterListener(this, &TestingComponent::DrawEventsListen);
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
  }
  WRITE(std::to_string(updateEvent->elapsedTime));
}

void TestingComponent::DrawEventsListen(DrawEvent* drawEvent) { }