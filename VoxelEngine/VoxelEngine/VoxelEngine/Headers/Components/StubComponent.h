/*HEADER_GOES_HERE*/
//Implimentation cost is minimum 6-7 lines of code for every member variable, 6 for default constructed components and 7 for non-default.
//    (in .h => declare, constructor declaration) (in .cpp => default arg in generate (if no default constructor), constructor declaration, constructor initialization, registration, cloning)
//Implimentation cost is minimum 4 lines of code for every event listener
//    (in .h => declarion) (in .cpp => registration, unregistration, declaration)
//Implimentation cost is minimum 1 line of code for every event pushed, see example by hitting f12 on WRITE

//additional cost is potential using get/set functions for non POD data members.
//Benefit is psuedo RTTR, serialization abstraction, undo/redo stack, gui editor abstraction, change logging/serialization, heuristics, and not having to open another file besides these 2
//for every member, with no effort except telling programmer providing this service to account for adding new types to each of the forementioned abilities.
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
  static const ID componentType = 0;

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