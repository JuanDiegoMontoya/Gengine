/*HEADER_GOES_HERE*/
#ifndef DestroyEvent_Guard
#define DestroyEvent_Guard

#include "Event.h"

#include <any>
#include <string>
#include <vector>
#include <boost/preprocessor/cat.hpp>

#define pDestroyEvent std::unique_ptr<DestroyEvent>;

class DestroyEvent : public Event    //Event containing frame data
{
public:
  static const ID eventType = cDestroyEvent;
  std::string GetName() { return "DestroyEvent"; }
  std::unique_ptr<Event> Clone() { return std::unique_ptr<Event>(new DestroyEvent(*this)); }
  /////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////// ___IGNORE EVERYTHING ABOVE___ ///////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////

  //starts at 256 as 0-255 are reserved for systems
  enum What
  {
    cEngine = 256,
    cObject = 257,
    cSpace = 258,
    cNothing = 259
  };

  ID what;
  std::any toDestroy;
  static std::unique_ptr<DestroyEvent> GenerateDestroyEvent(
    std::any toDestroy_, ID what_, float timer_ = HANDLE_INSTANTLY
  )
  {
    return std::unique_ptr<DestroyEvent>(new DestroyEvent(
      timer_, toDestroy_, what_
    ));
  }
  static std::unique_ptr<DestroyEvent> RegisterDestroyEvent()
  {
    return std::unique_ptr<DestroyEvent>(new DestroyEvent(
      HANDLE_INSTANTLY, nullptr, What::cNothing
    ));
  }

private:
  DestroyEvent(
    float timer_, std::any toDestroy_, ID what_
  ) : Event(cDestroyEvent, timer_), toDestroy(toDestroy_), what(what_)
  { }

  friend void RegisterEvents();
};

#endif //DestroyEvent_Guard