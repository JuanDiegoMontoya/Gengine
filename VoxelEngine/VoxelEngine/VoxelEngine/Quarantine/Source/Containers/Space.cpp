/*HEADER_GOES_HERE*/
#include "../../Headers/Containers/Space.h"
#include "../../Headers/Containers/Object.h"
#include "../../Headers/Factory.h"
#include "../../Headers/Engine.h"

#include "../../Headers/Events/UpdateEvent.h"
#include "../../Headers/Events/DrawEvent.h"
#include "../../Headers/Events/DestroyEvent.h"
#include "../../Headers/Events/InitEvent.h"

#include <algorithm>
#include <iostream>

  //Hacky bad for now stuff

#include "../../Headers/Systems/TestingSystem.h"
#include <Systems/VoxelWorld.h>
#include <Systems/Camera.h>
#include <Managers/GraphicsManager.h>

  //done with bad stuff

std::unique_ptr<Space> Space::CreateInitialSpace()
{
  //Hacky bad for now stuff
  std::unique_ptr<Space> newSpace = (*Factory::emptySpace).Clone();

  newSpace->name = std::string("TestingSpace");

  Object* newObject = Factory::CloneObject(&*newSpace);
  newObject->AttachSystem<TestingSystem>(CLONE_SYSTEM(TestingSystem));
  newObject->AttachSystem<VoxelWorld>(CLONE_SYSTEM(VoxelWorld));

  auto FUCK = std::make_unique<Camera>();
  FUCK->GenProjection(80.0f);
  GraphicsManager::GetGraphicsManager()->SetActiveCamera(FUCK.get());
  newObject->AttachSystem<Camera>(std::move(FUCK));

  return newSpace;

  //done with bad stuff
}

Space::Space(const std::string name_) : name(name_)
{
  static bool registered = false;
  if (!registered)
  {
    Factory::SpaceProperties = std::vector<PropertyID>({
      PROPERTY_ID(String, name),
      PROPERTY_ID(Float, zLayer),
      PROPERTY_ID(Bool, paused),
      PROPERTY_ID(Bool, isVisable)
      });
    registered = true;
  }
}

Space::~Space()
{
}

//void AttatchObjectHelper(Space* space, Object* object, bool initOnAttach)
//{
//  object->space = space;
//
//  if (initOnAttach)
//    object->Init();
//
//  for (auto i : object->GetChildren())
//    AttatchObjectHelper(space, i, initOnAttach);
//}
//
//Object* Space::AttachObject(std::unique_ptr<Object> objectToAdd, bool initOnAttach)
//{
//  //Object(const std::string &name_, const Object* otherObject, Space* const space_, bool initOnCreate = true);
//  if (objectToAdd->GetSpace() != nullptr) throw("Can't add object that's already in another space");
//  if (objectToAdd->GetParent() != nullptr) throw("Object added to space must be root object");
//
//  objects.push_back(std::move(objectToAdd));
//  auto addedObject = &*objects.back();
//
//  AttatchObjectHelper(this, addedObject, initOnAttach);
//
//  return addedObject;
//}

void Space::Init()
{
  Engine::GetEngine()->RegisterListener(this, &Space::Update);
  RegisterListener(this, &Space::DestroyEventsListen);

  std::unique_ptr<InitEvent> initEvent = InitEvent::GenerateInitEvent();

  for (auto& i : objects)
    i->Init(&*initEvent);

  hasInitialized = true;
}

std::unique_ptr<Space> Space::Clone()
{
  auto result = new Space(name + std::string("(clone)"));
  result->zLayer = zLayer;

  for (auto& i : objects)
  {
    if (i->GetRoot() == &*i)
    {
      i->Clone(result);
    }
    //result->objects.push_back(std::move(i->Clone()));
    //auto& addedObject = result->objects.back();
  }

  return std::unique_ptr<Space>(result);
}

void Space::Update(UpdateEvent* updateEvent)
{
  eventManager->AttachEvent(updateEvent->Clone());

  //Dispatch the events
  eventManager->Update(updateEvent->dt);
}

void Space::Draw(DrawEvent* drawEvent)
{
  if (isVisable)
    eventManager->AttachEvent(drawEvent->Clone());
}

void Space::DestroyEventsListen(DestroyEvent* destroyEvent)
{
  ASSERT(0);
  //if (destroyEvent->what == DestroyEvent::cSpace)
  //  RemoveObject(std::any_cast<Object*>(destroyEvent->toDestroy));
}

void Space::End()
{
  Engine::GetEngine()->UnregisterListener(this, &Space::Update);
  UnregisterListener(this, &Space::DestroyEventsListen);

  for (auto& i : objects)
    i->End();
}

Object* Space::FindObject(std::string objectName)
{
  for (auto& i : objects)
    if (i->GetName() == objectName)
      return &*i;
  return nullptr;
}

bool Space::ContainsObject(Object* object)
{

  //Find where it's at with binary search
  auto position = std::find(objects.begin(), objects.end(), std::unique_ptr<Object>(object));

  //Tell if value was found
  if (position == objects.end())
    return false;
  else
    return true;
}

std::vector<std::unique_ptr<Object>>& Space::GetObjects()
{
  return objects;
}

bool Space::HasInitialized()
{
  return hasInitialized;
}


#include <Containers/Entity.h>
//#include <Systems/Transform.h>
Entity Space::CreateEntity(std::string_view name)
{
  Entity entity(registry.create(), this);
  auto& tag = entity.AddSystem<Tag>();
  tag.tag = name.empty() ? "Entity" : name;
  return entity;
}

void Space::RemoveEntity(std::string_view ObjectName)
{
}

void Space::RemoveEntity(Entity entity)
{
}