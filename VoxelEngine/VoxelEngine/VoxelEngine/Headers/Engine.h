/*HEADER_GOES_HERE*/
#ifndef Engine_DEF
#define Engine_DEF

#include "Systems/System.h"
#include "../Headers/Containers/EventManger.h"
#include "FactoryID.h"
#include <list>
#include <memory>

typedef class Space Space;

class Engine
{
public:
  static const FactoryID factoryID = FactoryID::cEngine;

  static Engine* const GetEngine() { SINGLETON(Engine, engine); }

  ~Engine();

  void Init();
  void Update();
  void End();

  Space* AttachSpace(std::unique_ptr<Space>& spaceToAttach, bool initOnAttach = true);

  float ElapsedTime() { return elapsedTime; }

  template <typename T, typename E>
  void RegisterListener(T* object_, void(T::* callbackFunction_)(E*)) { eventManager->RegisterListener(object_, callbackFunction_); }

  template <typename T, typename E>
  void UnregisterListener(T* object_, void(T::* callbackFunction_)(E*)) { eventManager->UnregisterListener(object_, callbackFunction_); }

  template <typename T>
  void AttachEvent(std::unique_ptr<T> eventToAttach) { eventManager->AttachEvent(std::move(eventToAttach)); }

  template <typename T>
  void AttachEventRef(std::unique_ptr<T>& eventToAttach) { eventManager->AttachEventRef(eventToAttach); }


  bool quitEngine = false;

private:

  static Engine* engine;

  Engine();

  std::unique_ptr<EventManager> eventManager = std::unique_ptr<EventManager>(new EventManager());
  std::deque<std::unique_ptr<Space>> spaces;
  std::deque<System*> systems;
  float elapsedTime = 0.0f;

};

#endif // !Engine_DEF