/*HEADER_GOES_HERE*/
#include "../../Headers/Components/VoxelWorld.h"
#include "../../Headers/Containers/Object.h"
#include "../../Headers/Containers/Space.h"
#include "../../Headers/Factory.h"

//#include "../../Headers/Events/UpdateEvent.h"
//#include "../../Headers/Events/DrawEvent.h"
#include <Systems/GraphicsSystem.h>
#include <Refactor/hud.h>
#include <World/chunk_manager.h>

std::string VoxelWorld::GetName() { return "VoxelWorld"; }

std::unique_ptr<VoxelWorld> VoxelWorld::RegisterVoxelWorld() 
{
  auto component = std::unique_ptr<VoxelWorld>(new VoxelWorld(
    //5999                                            /***Only needed if you don't provide a default constructor***/
  ));

  return std::move(component);
}

VoxelWorld::VoxelWorld(
  //Int componentData_
) : Component(componentType) //, componentData(componentData_)
{
}

VoxelWorld::~VoxelWorld()
{
}

void VoxelWorld::Init()
{
  //GetSpace()->RegisterListener(this, &VoxelWorld::UpdateEventsListen);
  //GetSpace()->RegisterListener(this, &VoxelWorld::DrawEventsListen);
}

void VoxelWorld::End()
{
  if (parent != nullptr)
  {
  //  GetSpace()->UnregisterListener(this, &VoxelWorld::UpdateEventsListen);
  //  GetSpace()->UnregisterListener(this, &VoxelWorld::DrawEventsListen);
  }
}

std::unique_ptr<Component> VoxelWorld::Clone() const
{
  auto result = new VoxelWorld();
    //copy over values here
  //result->componentData = componentData;
  assert(false); // do not copy this shit
  return std::unique_ptr<Component>(result);
}

//void VoxelWorld::UpdateEventsListen(UpdateEvent* updateEvent) { }

//void VoxelWorld::DrawEventsListen(DrawEvent* drawEvent) { }