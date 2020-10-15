/*HEADER_GOES_HERE*/
#ifndef StubManager_DEF
#define StubManager_DEF

#include "Manager.h"
#include "../FactoryID.h"

class StubManager : public Manager
{
public:

  static const ID managerType = cStubManager;

  static StubManager* pStubManager;

  static StubManager* const GetStubManager() { SINGLETON(StubManager, pStubManager); }

  ~StubManager();
  void Init();
  void End();
  std::string GetName();

private:
  StubManager();

  friend void RegisterManagers();
};


#endif // !StubManager_DEF