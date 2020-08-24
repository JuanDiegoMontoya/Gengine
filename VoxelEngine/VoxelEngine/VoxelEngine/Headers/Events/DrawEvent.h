/*HEADER_GOES_HERE*/
#ifndef DrawEvent_Guard
#define DrawEvent_Guard

#include "Event.h"

#include <iostream>
#include <boost/preprocessor/cat.hpp>

#define pDrawEvent std::unique_ptr<DrawEvent>;

class DrawEvent : public Event    //Event containing frame data
{
public:
  static const ID eventType = cDrawEvent;
  std::string GetName() { return "DrawEvent"; }
  std::unique_ptr<Event> Clone() { return std::unique_ptr<Event>(new DrawEvent(*this)); }
  /////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////// ___IGNORE EVERYTHING ABOVE___ ///////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  //Implimentation cost is minimum 5-6 lines of code for every member variable; 
  //declaration, generation arg, constructor forwarding, registration(only needed if no default constructor), constructor arg, and constructor init


  //int eventData;
  static std::unique_ptr<DrawEvent> GenerateDrawEvent(
    /*int eventData_ = 59999, */float timer_ = HANDLE_INSTANTLY  //you can change default to a delay (zero is next frame), or remove the user ability to change delay. "= 5999" is optional default
  )
  {
    return std::unique_ptr<DrawEvent>(new DrawEvent(
      /*eventData_, */ timer_
    ));
  }
  static std::unique_ptr<DrawEvent> RegisterDrawEvent()
  {
    return std::unique_ptr<DrawEvent>(new DrawEvent(
      /*59999,  HANDLE_INSTANTLY */                         //Only needed if you don't have default constructor
    ));
  }


private:

  DrawEvent(
    /*int eventData_, */ float timer_ = HANDLE_INSTANTLY            //again you can change default delay to whatever you want
  ) : Event(cDrawEvent, timer_) //, eventData(eventData_)
  { }

  friend void RegisterEvents();
};

#endif //DrawEvent_Guard