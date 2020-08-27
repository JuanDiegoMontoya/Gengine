/*HEADER_GOES_HERE*/
#include "../../Headers/Components/Component.h"
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
  for (int i = 0; i < COMPONENT_COUNT; i++)
    if (components[i])
    {
      DetatchComponent(components[i]->type);
    }
  RemoveParent();
  RemoveAllChildren();
}

void Object::DetatchComponent(ID type)
{
  if (components[type] != nullptr)
  {
    components[type]->End();
    components[type] = nullptr;
  }
}

Object& Object::operator=(const Object& rhs)
{
  //Remove all old components
  for (auto& i : components)
    if (i != nullptr)
      DetatchComponent(i->type);

  //Add all new components
  for (auto& i : rhs.components)
    if (i != nullptr)
      AttachComponent(i->Clone());

  return *this;
}

Component* Object::FindComponent(ID componentType)
{
  return &*components[componentType];
}

Component* Object::AttachComponent(std::unique_ptr<Component> componentToAttach)
{
  if(componentToAttach->GetParent() != nullptr) throw("Can't add component that's already on another object");
  DetatchComponent(componentToAttach->type);
  componentToAttach->parent = this;
  if (GetSpace()->HasInitialized())
    componentToAttach->Init();
  return &*(components[componentToAttach->type] = std::move(componentToAttach));
}

void Object::Init(InitEvent* initEvent)
{
  space->RegisterListener(this, &Object::UpdateEventsListen);
  for (int i = 0; i < components.size(); ++i)
    if (components[i] != nullptr)
      components[i]->Init();
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

  for (auto& i : components)
    if (i != nullptr)
    {
      result->components[i->type]->parent = this;
      &*(components[i->type] = std::move(i->Clone()));
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
  for (auto& i : components)
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

void Object::PerfromOnAllChildrenAndSelf(std::function<void(Object*)> f)
{
  f(this);
  PerfromOnAllChildren(f);
}

void Object::PerfromOnAllChildren(std::function<void(Object*)> f)
{
  for (auto child : children)
  {
    f(child);
    child->PerfromOnAllChildren(f);
  }
}