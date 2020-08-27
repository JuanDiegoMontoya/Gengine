/*HEADER_GOES_HERE*/
#ifndef TraceSystem_DEF
#define TraceSystem_DEF

#include "System.h"
#include "../FactoryID.h"

typedef class TraceEvent TraceEvent;

class TraceSystem : public System
{
public:

  static const ID systemType = cTraceSystem;

  static TraceSystem* pTraceSystem;

  static TraceSystem* const GetTraceSystem() { SINGLETON(TraceSystem, pTraceSystem); }

  void TraceEventsListen(TraceEvent* traceEvent);

  ~TraceSystem();
  void Init();
  void End();
  std::string GetName();

private:
  TraceSystem();

  friend void RegisterSystems();
};


#endif // !TraceSystem_DEF