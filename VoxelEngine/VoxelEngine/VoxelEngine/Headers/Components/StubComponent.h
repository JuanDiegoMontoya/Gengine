#ifndef StubComponent_Guard
#define StubComponent_Guard

#include <iostream>
#include "../Containers/Properties.h"
#include "Component.h"

//typedef class UpdateEvent UpdateEvent;
//typedef class DrawEvent DrawEvent;


class StubComponent : public Component
{
public:
  static const ID componentType = cStubComponent;

  //PROPERTY(Int, componentData, 5999);

  StubComponent(
    //Int componentData_ = 5999                                             /*** "= 5999" Only needed if you want a default constructor***/
  );
  ~StubComponent();
  void Init();
  void End();

  std::unique_ptr<Component> Clone() const;
  std::string GetName();

  //void UpdateEventsListen(UpdateEvent* updateEvent);
  //void DrawEventsListen(DrawEvent* drawEvent);

  static std::unique_ptr<StubComponent> RegisterStubComponent();

private:

  friend void RegisterComponents();
};

#endif // !StubComponent_Guard