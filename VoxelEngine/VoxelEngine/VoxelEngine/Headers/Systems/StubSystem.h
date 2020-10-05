#ifndef StubSystem_Guard
#define StubSystem_Guard

#include <iostream>
#include "../Containers/Properties.h"
#include "System.h"

//typedef class UpdateEvent UpdateEvent;
//typedef class DrawEvent DrawEvent;


class StubSystem : public System
{
public:
  static const ID systemType = cStubSystem;

  //PROPERTY(Int, systemData, 5999);

  StubSystem(
    //Int systemData_ = 5999                                             /*** "= 5999" Only needed if you want a default constructor***/
  );
  ~StubSystem();
  void Init();
  void End();

  std::unique_ptr<System> Clone() const;
  std::string GetName();

  //void UpdateEventsListen(UpdateEvent* updateEvent);
  //void DrawEventsListen(DrawEvent* drawEvent);

  static std::unique_ptr<StubSystem> RegisterStubSystem();

private:

  friend void RegisterSystems();
};

#endif // !StubSystem_Guard