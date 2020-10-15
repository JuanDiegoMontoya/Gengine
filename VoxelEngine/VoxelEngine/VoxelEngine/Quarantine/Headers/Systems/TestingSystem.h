#ifndef TestingSystem_Guard
#define TestingSystem_Guard

#include <iostream>
#include "../Containers/Properties.h"
#include "System.h"

typedef class UpdateEvent UpdateEvent;
typedef class DrawEvent DrawEvent;
typedef class TestingEvent TestingEvent;


class TestingSystem : public System
{
public:
  static const ID systemType = cTestingSystem;

  PROPERTY(Int, systemData, 5999);

  TestingSystem(
    Int systemData_ = 5999                                             /*** "= 5999" Only needed if you want a default constructor***/
  );
  ~TestingSystem();
  void Init();
  void End();

  std::unique_ptr<System> Clone() const;
  std::string GetName();

  void UpdateEventsListen(UpdateEvent* updateEvent);
  void DrawEventsListen(DrawEvent* drawEvent);
  void TestingEventsListen(TestingEvent* testingEvent);

  static std::unique_ptr<TestingSystem> RegisterTestingSystem();

private:

  friend void RegisterSystems();
};

#endif // !TestingSystem_Guard