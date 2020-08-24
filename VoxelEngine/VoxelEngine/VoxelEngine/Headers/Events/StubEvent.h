/*HEADER_GOES_HERE*/
#ifndef StubEvent_Guard
#define StubEvent_Guard

#include "Event.h"

#include <iostream>
#include <boost/preprocessor/cat.hpp>

#define pStubEvent std::unique_ptr<StubEvent>;

class StubEvent : public Event    //Event containing frame data
{
public:
  static const ID eventType = cStubEvent;
  std::string GetName() { return "StubEvent"; }
  std::unique_ptr<Event> Clone() { return std::unique_ptr<Event>(new StubEvent(*this)); }
  /////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////// ___IGNORE EVERYTHING ABOVE___ ///////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  //Implimentation cost is minimum 5-6 lines of code for every member variable; 
  //declaration, generation arg, constructor forwarding, registration(only needs adjustment if no default constructor), constructor arg, and constructor init


  //PROPERTY(Int, eventData, 0);
  static std::unique_ptr<StubEvent> GenerateStubEvent(
    /*Int eventData_ = 59999, */float timer_ = HANDLE_INSTANTLY  //you can change default to a delay (zero is next frame), or remove the user ability to change delay. "= 5999" is optional default
  )
  {
    return std::unique_ptr<StubEvent>(new StubEvent(
      /*eventData_, */ timer_
    ));
  }
  static std::unique_ptr<StubEvent> RegisterStubEvent()
  {
    return std::unique_ptr<StubEvent>(new StubEvent(
      /*59999,  HANDLE_INSTANTLY */                         //Only needed if you don't have default constructor
    ));
  }


private:

  StubEvent(
    /*Int eventData_, */ float timer_ = HANDLE_INSTANTLY            //again you can change default delay to whatever you want
  ) : Event(cStubEvent, timer_) //, eventData(eventData_)
  { }

  friend void RegisterEvents();
};

#endif //StubEvent_Guard