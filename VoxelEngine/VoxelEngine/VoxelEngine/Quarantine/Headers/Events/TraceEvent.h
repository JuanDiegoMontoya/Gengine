/*HEADER_GOES_HERE*/
#ifndef TraceEvent_Guard
#define TraceEvent_Guard

#include "Event.h"
#include "../Engine.h"

#include <iostream>
#include <boost/preprocessor/cat.hpp>

#define pTraceEvent std::unique_ptr<TraceEvent>;

#define WRITE(message) { auto traceMessagex = TraceEvent::GenerateTraceEvent( message ); Engine::GetEngine()->AttachEventRef(traceMessagex); }

class TraceEvent : public Event    //Event containing frame data
{
public:
  static const ID eventType = cTraceEvent;
  std::string GetName() { return "TraceEvent"; }
  std::unique_ptr<Event> Clone() { return std::unique_ptr<Event>(new TraceEvent(*this)); }
  /////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////// ___IGNORE EVERYTHING ABOVE___ ///////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////////////////


  std::string traceMessage;
  static std::unique_ptr<TraceEvent> GenerateTraceEvent(
    std::string traceMessage_ = "", float timer_ = HANDLE_INSTANTLY
  )
  {
    return std::unique_ptr<TraceEvent>(new TraceEvent(
      traceMessage_, timer_
    ));
  }
  static std::unique_ptr<TraceEvent> RegisterTraceEvent()
  {
    return std::unique_ptr<TraceEvent>(new TraceEvent(
      "", HANDLE_INSTANTLY
    ));
  }

private:
  TraceEvent(
    std::string traceMessage_, float timer_
  ) : Event(cTraceEvent, timer_), traceMessage(traceMessage_)
  { }

  friend void RegisterEvents();
};

#endif //TraceEvent_Guard