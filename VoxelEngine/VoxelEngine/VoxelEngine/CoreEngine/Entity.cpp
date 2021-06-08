#include "EnginePCH.h"
#include "Entity.h"
#include "Components/Transform.h"
#include "Components/Core.h"

using namespace Component;


void Entity::Destroy()
{
  ASSERT_MSG(*this, "Cannot delete invalid entity!");

  // remove self from any existing parent
  if (auto parent = TryGetComponent<Component::Parent>())
  {
    parent->entity.GetComponent<Component::Children>().RemoveChild(*this);
  }

  GetOrAddComponent<Component::ScheduledDeletion>();
}

void Entity::SetParent(Entity parent)
{
  ASSERT(parent != *this);
  if (HasComponent<Parent>())
  {
    // remove self from old parent
    Entity oldParent = GetComponent<Parent>().entity;
    oldParent.GetComponent<Children>().RemoveChild(*this);
    if (oldParent.GetComponent<Children>().size() == 0)
      oldParent.RemoveComponent<Children>();
    GetComponent<Parent>().entity = parent;
  }
  else
  {
    AddComponent<Parent>(parent);
  }

  if (parent.HasComponent<Children>())
    parent.GetComponent<Children>().AddChild(*this);
  else
    parent.AddComponent<Children>().AddChild(*this);

  // recursively update all parents whose height is changed by the new subtree
  Entity tParent = parent;
  unsigned tChildHeight = HasComponent<Children>() ? GetComponent<Children>().cachedHeight : 0;
  while (tParent.HasComponent<Children>() && tParent.GetComponent<Children>().cachedHeight <= 1 + tChildHeight)
  {
    tParent.GetComponent<Children>().cachedHeight = ++tChildHeight;
    if (tParent.HasComponent<Parent>())
      tParent = tParent.GetComponent<Parent>().entity;
    else
      break;
  }

  {
    Entity cParent = parent;
    while (cParent.HasComponent<Parent>())
    {
      cParent = cParent.GetComponent<Parent>().entity;
      ASSERT_MSG(cParent != *this, "Parenting creates a cycle!");
    }
  }
}

void Entity::AddChild(Entity child)
{
  ASSERT(child != *this);
  child.SetParent(*this);
}

unsigned Entity::GetHierarchyHeight() const
{
  return HasComponent<Children>() ? GetComponent<Children>().cachedHeight : 0;
}