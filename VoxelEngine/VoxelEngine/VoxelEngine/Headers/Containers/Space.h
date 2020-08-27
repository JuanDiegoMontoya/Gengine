/*HEADER_GOES_HERE*/
#ifndef Space_DEF
#define Space_DEF

#include <string>
#include <vector>
#include <memory>

#include "EventManger.h"
#include "../FactoryID.h"

typedef class Engine Engine;
typedef class Object Object;

typedef class UpdateEvent UpdateEvent;
typedef class DrawEvent DrawEvent;
typedef class DestroyEvent DestroyEvent;




class Space
{
public:
  static const FactoryID factoryID = FactoryID::cSpace;

  static std::unique_ptr<Space> CreateInitialSpace();

  Space(const std::string name_);
  ~Space();

  void Init();
  void End();
  std::unique_ptr<Space> Clone();

  void Update(UpdateEvent* updateEvent);
  void Draw(DrawEvent* drawEvent);
  void DestroyEventsListen(DestroyEvent* destroyEvent);

    //TODO: make this a friend function of the object class
  //Object* AttachObject(std::unique_ptr<Object> ObjectToAdd, bool initOnAttach = true);
  Object* FindObject(std::string ObjectName);

  //tells if the space has the Object
  bool ContainsObject(Object* Object);
  const std::string& GetName() const { return name; }
  void SetName(std::string name_) { name = name_; }
  const std::vector<std::unique_ptr<Object>>& GetEntities() { return objects; }
  void RemoveObject(std::string ObjectName);
  void RemoveObject(Object* Object);
  float GetZLayer() { return zLayer; }

  void SetZLayer(float zLayer_) { zLayer = zLayer_; }

  template <typename T> void AttachEvent(std::unique_ptr<T> eventToAttach) { eventManager->AttachEvent(std::move(eventToAttach)); }
  template <typename T> void AttachEventRef(std::unique_ptr<T>& eventToAttach) { eventManager->AttachEventRef(eventToAttach); }
  //Registers an object with a listener to the event manager.
  template <typename T, typename E> void RegisterListener(T* object_, void(T::* callbackFunction_)(E*)) { eventManager->RegisterListener(object_, callbackFunction_); }
  //Unregisters an object with a listener from the event manager.
  template <typename T, typename E>  void UnregisterListener(T* object_, void(T::* callbackFunction_)(E*)) { eventManager->UnregisterListener(object_, callbackFunction_); }

    //Toggle to affect update events being dispatched.
  PROPERTY(Bool, paused, false);
  PROPERTY(Bool, isVisable, true);

  std::vector<std::unique_ptr<Object>>& GetObjects();

  bool HasInitialized();

private:

  bool hasInitialized = false;

  std::unique_ptr<EventManager> eventManager = std::unique_ptr<EventManager>(new EventManager());

  std::vector<std::unique_ptr<Object>> objects;

  PROPERTY(String, name, std::string("empty"));
  PROPERTY(Float, zLayer, 0.f);
};


#endif // !Space_DEF