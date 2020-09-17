/*HEADER_GOES_HERE*/
#ifndef Object_Guard
#define Object_Guard

#include <array>
#include <memory>

#include "EventManger.h"
#include "Space.h"
//#include "../Engine.h"
#include "../Components/Component.h"
#include "../FactoryID.h"
#include <string>
#include <memory>
#include <entt.hpp>


typedef class InitEvent InitEvent;
typedef class UpdateEvent UpdateEvent;
typedef class Space Space;



class Object
{
public:
  static const FactoryID factoryID = FactoryID::cObject;

  Object(const std::string& emptyName_);
  ~Object();
  //Object(const std::string& name_, const std::string& archetypeName_, Space* const space_);

  Object& operator=(const Object& rhs);

  void Init(InitEvent* initEvent);
  void End();

  Object* Clone(Space* space);

  void UpdateEventsListen(UpdateEvent* updateEvent);

  std::string GetName() const { return name; }
  void SetName(std::string name_) { name = name_; }
  Space* GetSpace() const { return space; }
  template <typename T> bool Has() { return FindComponent(T::componentType) != nullptr; }
  bool Has(ID componentType) { return FindComponent(componentType) != nullptr; }

    //Will not throw error if component not found; instead will return null
  template <typename T> T* Get() { return reinterpret_cast<T*>(FindComponent(T::componentType)); }
    //Will throw an error if the Object doesn't have the needed component
  template <typename T> T* GetRequired() { return reinterpret_cast<T*>(FindRequiredComponent(T::componentType)); }

  Component* AttachComponent(std::unique_ptr<Component> componentToAttach);

  template<typename T>
  T* AttachComponent(std::unique_ptr<T> componentToAttach)
  {
    if (componentToAttach->GetParent() != nullptr) throw("Can't add component that's already on another object");
    DetatchComponent(componentToAttach->type);
    componentToAttach->parent = this;
    if (GetSpace()->HasInitialized())
      componentToAttach->Init();
    auto componentToAttachType = componentToAttach->type;
    return reinterpret_cast<T*>(&*(components[componentToAttachType] = std::unique_ptr<Component>(static_cast<T*>(componentToAttach.release()))));
  }

  void DetatchComponent(ID type);

  void ParentTo(Object* parent_);
  void AddChild(Object* child_);

  Object* GetRoot();
  bool IsRoot();
  
  Object* GetParent();
  const std::deque<Object*>& GetChildren();
 
  void RemoveParent();
  void RemoveChild(Object* child_);
  void RemoveAllChildren();

  void PerformOnAllChildrenAndSelf(std::function<void(Object*)> f);
  void PerformOnAllChildren(std::function<void(Object*)> f);

  template <typename T> void AttachEvent(std::unique_ptr<T> eventToAttach) { eventManager->AttachEvent(std::move(eventToAttach)); }
  template <typename T> void AttachEventRef(std::unique_ptr<T>& eventToAttach) { eventManager->AttachEventRef(eventToAttach); }
  //Registers an object with a listener to the event manager.
  template <typename T, typename E> void RegisterListener(T* object_, void(T::* callbackFunction_)(E*)) { eventManager->RegisterListener(object_, callbackFunction_); }
  //Unregisters an object with a listener from the event manager.
  template <typename T, typename E> void UnregisterListener(T* object_, void(T::* callbackFunction_)(E*)) { eventManager->UnregisterListener(object_, callbackFunction_); }

  Component* FindComponent(ID componentType);

  PROPERTY(Bool, paused, false);
  PROPERTY(Bool, isTemp, false);

  std::unique_ptr<EventManager> eventManager = std::unique_ptr<EventManager>(new EventManager());

  Space* space = nullptr;

private:

  Object* parent = nullptr;
  std::deque<Object*> children = std::deque<Object*>();

  PROPERTY(String, name, std::string("empty"));

  std::array<std::unique_ptr<Component>, COMPONENT_COUNT> components = { { 0 } };

};


#endif // !Object_Guard