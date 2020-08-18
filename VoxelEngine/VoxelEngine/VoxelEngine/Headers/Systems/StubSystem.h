/*HEADER_GOES_HERE*/
#ifndef StubSystem_DEF
#define StubSystem_DEF

#include "System.h"
#include "../FactoryID.h"

class StubSystem : public System
{
public:

  static StubSystem* pStubSystem;

  static StubSystem* const GetStubSystem() { SINGLETON(StubSystem, pStubSystem); }

  ~StubSystem();
  void Init();
  void End();
  std::string GetName();

private:
  StubSystem();

  friend void RegisterSystems();
};


#endif // !StubSystem_DEF