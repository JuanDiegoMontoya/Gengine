/*HEADER_GOES_HERE*/
#ifndef UpdateEvent_Guard
#define UpdateEvent_Guard

#include "Event.h"

#include <iostream>
#include <boost/preprocessor/cat.hpp>

#define pUpdateEvent std::unique_ptr<UpdateEvent>;

class UpdateEvent : public Event    //Event containing frame data
{
public:
  static const ID eventType = cUpdateEvent;
  std::string GetName() { return "UpdateEvent"; }
  std::unique_ptr<Event> Clone() { return std::unique_ptr<Event>(new UpdateEvent(*this)); }
  /////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////// ___IGNORE EVERYTHING ABOVE___ ///////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  //Implimentation cost is minimum 5-6 lines of code for every member variable; 
  //declaration, generation arg, constructor forwarding, registration(only needed if no default constructor), constructor arg, and constructor init


  PROPERTY(Float, dt, 0.f);
  PROPERTY(Float, elapsedTime, 0.f);
  static std::unique_ptr<UpdateEvent> GenerateUpdateEvent(
    Float dt_, Float elaspedTime_, float timer_ = HANDLE_INSTANTLY  //you can change default to a delay (zero is next frame), or remove the user ability to change delay. "= 5999" is optional default
  )
  {
    return std::unique_ptr<UpdateEvent>(new UpdateEvent(
      dt_, elaspedTime_, timer_
    ));
  }
  static std::unique_ptr<UpdateEvent> RegisterUpdateEvent()
  {
    return std::unique_ptr<UpdateEvent>(new UpdateEvent(
      0.f, 0.f,  HANDLE_INSTANTLY                         //Only needed if you don't have default constructor
    ));
  }


private:

  UpdateEvent(
    Float dt_, Float elaspedTime_, float timer_ = HANDLE_INSTANTLY            //again you can change default delay to whatever you want
  ) : Event(cUpdateEvent, timer_), dt(dt_), elapsedTime(elaspedTime_)
  { }

  friend void RegisterEvents();
};

#endif //UpdateEvent_Guard