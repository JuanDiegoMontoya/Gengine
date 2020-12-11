/*HEADER_GOES_HERE*/
#ifndef InitEvent_Guard
#define InitEvent_Guard

#include "Event.h"

#include <iostream>
#include <boost/preprocessor/cat.hpp>

#define pInitEvent std::unique_ptr<InitEvent>;

class InitEvent : public Event    //Event containing frame data
{
public:
  static const ID eventType = cInitEvent;
  std::string GetName() { return "InitEvent"; }
  std::unique_ptr<Event> Clone() { return std::unique_ptr<Event>(new InitEvent(*this)); }
  /////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////// ___IGNORE EVERYTHING ABOVE___ ///////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  //Implimentation cost is minimum 5-6 lines of code for every member variable; 
  //declaration, generation arg, constructor forwarding, registration(only needs adjustment if no default constructor), constructor arg, and constructor init


  //PROPERTY(Int, eventData, 0);
  static std::unique_ptr<InitEvent> GenerateInitEvent(
    /*Int eventData_ = 59999, */float timer_ = HANDLE_INSTANTLY  //you can change default to a delay (zero is next frame), or remove the user ability to change delay. "= 5999" is optional default
  )
  {
    return std::unique_ptr<InitEvent>(new InitEvent(
      /*eventData_, */ timer_
    ));
  }
  static std::unique_ptr<InitEvent> RegisterInitEvent()
  {
    return std::unique_ptr<InitEvent>(new InitEvent(
      /*59999,  HANDLE_INSTANTLY */                         //Only needed if you don't have default constructor
    ));
  }


private:

  InitEvent(
    /*Int eventData_, */ float timer_ = HANDLE_INSTANTLY            //again you can change default delay to whatever you want
  ) : Event(cInitEvent, timer_) //, eventData(eventData_)
  { }

  friend void RegisterEvents();
};

#endif //InitEvent_Guard