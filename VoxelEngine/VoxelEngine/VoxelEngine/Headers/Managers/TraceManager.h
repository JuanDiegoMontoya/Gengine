/*HEADER_GOES_HERE*/
#ifndef TraceManager_DEF
#define TraceManager_DEF

#include "Manager.h"
#include "../FactoryID.h"

typedef class TraceEvent TraceEvent;

class TraceManager : public Manager
{
public:

  static const ID managerType = cTraceManager;

  static TraceManager* pTraceManager;

  static TraceManager* const GetTraceManager() { SINGLETON(TraceManager, pTraceManager); }

  void TraceEventsListen(TraceEvent* traceEvent);

  ~TraceManager();
  void Init();
  void End();
  std::string GetName();

private:
  TraceManager();

  friend void RegisterManagers();
};


#endif // !TraceManager_DEF