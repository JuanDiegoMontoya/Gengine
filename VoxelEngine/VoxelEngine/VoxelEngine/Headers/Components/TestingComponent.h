#ifndef TestingComponent_Guard
#define TestingComponent_Guard

#include <iostream>
#include "../Containers/Properties.h"
#include "Component.h"

typedef class UpdateEvent UpdateEvent;
typedef class DrawEvent DrawEvent;
typedef class TestingEvent TestingEvent;


class TestingComponent : public Component
{
public:
  static const ID componentType = cTestingComponent;

  PROPERTY(Int, componentData, 5999);

  TestingComponent(
    Int componentData_ = 5999                                             /*** "= 5999" Only needed if you want a default constructor***/
  );
  ~TestingComponent();
  void Init();
  void End();

  std::unique_ptr<Component> Clone() const;
  std::string GetName();

  void UpdateEventsListen(UpdateEvent* updateEvent);
  void DrawEventsListen(DrawEvent* drawEvent);
  void TestingEventsListen(TestingEvent* testingEvent);

  static std::unique_ptr<TestingComponent> RegisterTestingComponent();

private:

  friend void RegisterComponents();
};

#endif // !TestingComponent_Guard