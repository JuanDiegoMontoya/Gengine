/*HEADER_GOES_HERE*/
#include "../../Headers/Systems/System.h"
#include "../../Headers/Containers/Space.h"
#include "../../Headers/Engine.h"
#include "../../Headers/Factory.h"
#include "../../Headers/Containers/Object.h"


#include "../../Headers/Events/InitEvent.h"
#include "../../Headers/Events/UpdateEvent.h"

Object::Object(const std::string& emptyName_) : name(emptyName_), space(nullptr)
{
  static bool registered = false;
  if (!registered)
  {
    Factory::SpaceProperties = std::vector<PropertyID>({
      PROPERTY_ID(String, name),
      PROPERTY_ID(Bool, isTemp),
      PROPERTY_ID(Bool, paused)
      });
    registered = true;
  }
}

Object::~Object()
{
  for (int i = 0; i < SYSTEM_COUNT; i++)
    if (systems[i])
    {
      DetatchSystem(systems[i]->type);
    }
  RemoveParent();
  RemoveAllChildren();
}

void Object::DetatchSystem(ID type)
{
  if (systems[type] != nullptr)
  {
    systems[type]->End();
    systems[type] = nullptr;
  }
}

Object& Object::operator=(const Object& rhs)
{
  //Remove all old systems
  for (auto& i : systems)
    if (i != nullptr)
      DetatchSystem(i->type);

  //Add all new systems
  for (auto& i : rhs.systems)
    if (i != nullptr)
      AttachSystem(i->Clone());

  return *this;
}

System* Object::FindSystem(ID systemType)
{
  return &*systems[systemType];
}

System* Object::AttachSystem(std::unique_ptr<System> systemToAttach)
{
  if(systemToAttach->GetParent() != nullptr) throw("Can't add system that's already on another object");
  DetatchSystem(systemToAttach->type);
  systemToAttach->parent = this;
  if (GetSpace()->HasInitialized())
    systemToAttach->Init();
  return &*(systems[systemToAttach->type] = std::move(systemToAttach));
}

void Object::Init(InitEvent* initEvent)
{
  space->RegisterListener(this, &Object::UpdateEventsListen);
  for (int i = 0; i < systems.size(); ++i)
    if (systems[i] != nullptr)
      systems[i]->Init();
}

void CloneHelper(Object* object, Space* space)
{

}

Object* Object::Clone(Space* space_)
{
  if (space_ == nullptr)
    throw("an object needs a space to be stored in");
  auto result = std::unique_ptr<Object>(new Object(name + std::string("(copy)")));
  auto resultPointer = &*result;
  //copy over values here
  result->isTemp = isTemp;
  result->paused = paused;

  for (auto& i : systems)
    if (i != nullptr)
    {
      result->systems[i->type]->parent = this;
      &*(systems[i->type] = std::move(i->Clone()));
    }

  space_->GetObjects().push_back(std::move(result));
  resultPointer->space = space_;

  for (auto i : children)
  {
    Object* child = i->Clone(space_);
    resultPointer->AddChild(child);
  }

  return resultPointer;
}

void Object::End()
{
  if (space != nullptr)
  {
    space->UnregisterListener(this, &Object::UpdateEventsListen);
  }
  for (auto& i : systems)
    if (i != nullptr)
      i->End();
}

void Object::UpdateEventsListen(UpdateEvent* updateEvent)
{
  //If the space isn't paused, and the individual object isn't paused..
  if (!space->paused && !paused)
    eventManager->AttachEvent(updateEvent->Clone());

  eventManager->Update(updateEvent->dt);

}


void Object::ParentTo(Object* parent_)
{
  if (parent != nullptr)
    parent->RemoveChild(this);
  parent = parent_;
  if (parent != nullptr)
    parent->children.push_back(this);
}
void Object::AddChild(Object* child_)
{
  child_->ParentTo(this);
}

Object* Object::GetParent()
{
  return parent;
}
const std::deque<Object*>& Object::GetChildren()
{
  return children;
}

Object* Object::GetRoot()
{
  Object* walker = this;
  while (walker->parent != nullptr)
  {
    walker = walker->parent;
  }
  return walker;
}
bool Object::IsRoot()
{
  return parent == nullptr;
}

void Object::RemoveParent()
{
  if (parent != nullptr)
  {
    parent->RemoveChild(this);
  }
}
void Object::RemoveChild(Object* child_)
{
  std::deque<Object*>::iterator child = std::find(children.begin(), children.end(), child_);
  if (child != children.end())
  {
    (*child)->parent = nullptr;
    children.erase(child);
  }
}
void Object::RemoveAllChildren()
{
  for (auto i : children)
  {
    i->parent = nullptr;
  }
  children.clear();
}

void Object::PerformOnAllChildrenAndSelf(std::function<void(Object*)> f)
{
  f(this);
  PerformOnAllChildren(f);
}

void Object::PerformOnAllChildren(std::function<void(Object*)> f)
{
  for (auto child : children)
  {
    f(child);
    child->PerformOnAllChildren(f);
  }
}