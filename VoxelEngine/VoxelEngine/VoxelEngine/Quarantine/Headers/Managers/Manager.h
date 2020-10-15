/*HEADER_GOES_HERE*/
#ifndef Manager_Guard
#define Manager_Guard

#include "../FactoryID.h"
#include <string>
#include <boost/preprocessor/cat.hpp>

#define SINGLETON(CLASSTYPE, CLASSPTR) if(CLASSPTR == nullptr) { CLASSPTR = new CLASSTYPE(); } return CLASSPTR;

//#define GetManager(Manager) BOOST_PP_CAT(Manager, BOOST_PP_CAT(::, BOOST_PP_CAT(Get, BOOST_PP_CAT(Manager, ()))))

class Manager
{
public:
  static const FactoryID factoryID = FactoryID::cManager;

  virtual std::string GetName() = 0;

  virtual void Init() = 0;
  virtual void End() = 0;

};

#endif // !Manager_Guard
