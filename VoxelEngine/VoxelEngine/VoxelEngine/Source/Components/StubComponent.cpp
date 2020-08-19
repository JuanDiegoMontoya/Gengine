/*HEADER_GOES_HERE*/
#include "../../Headers/Components/StubComponent.h"
#include "../../Headers/Containers/Object.h"
#include "../../Headers/Containers/Space.h"
#include "../../Headers/Factory.h"

//#include "../../Headers/Events/UpdateEvent.h"
//#include "../../Headers/Events/DrawEvent.h"

std::string StubComponent::GetName() { return "StubComponent"; }

std::unique_ptr<StubComponent> StubComponent::RegisterStubComponent() 
{
  auto component = std::unique_ptr<StubComponent>(new StubComponent(
    //5999                                            /***Only needed if you don't provide a default constructor***/
  ));

  return std::move(component);
}

StubComponent::StubComponent(
  //Int componentData_
) : Component(componentType) //, componentData(componentData_)
{
}

StubComponent::~StubComponent()
{
}

void StubComponent::Init()
{
  //GetSpace()->RegisterListener(this, &StubComponent::UpdateEventsListen);
  //GetSpace()->RegisterListener(this, &StubComponent::DrawEventsListen);
}

void StubComponent::End()
{
  if (parent != nullptr)
  {
  //  GetSpace()->UnregisterListener(this, &StubComponent::UpdateEventsListen);
  //  GetSpace()->UnregisterListener(this, &StubComponent::DrawEventsListen);
  }
}

std::unique_ptr<Component> StubComponent::Clone() const
{
  auto result = new StubComponent();
    //copy over values here
  //result->componentData = componentData;

  return std::unique_ptr<Component>(result);
}

//void StubComponent::UpdateEventsListen(UpdateEvent* updateEvent) { }

//void StubComponent::DrawEventsListen(DrawEvent* drawEvent) { }