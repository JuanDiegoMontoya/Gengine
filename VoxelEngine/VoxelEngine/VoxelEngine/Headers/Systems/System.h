/*HEADER_GOES_HERE*/
#ifndef System_Guard
#define System_Guard

#include "../FactoryID.h"
#include <string>
#include <boost/preprocessor/cat.hpp>

#define SINGLETON(CLASSTYPE, CLASSPTR) if(CLASSPTR == nullptr) { CLASSPTR = new CLASSTYPE(); } return CLASSPTR;

//#define GetSystem(system) BOOST_PP_CAT(system, BOOST_PP_CAT(::, BOOST_PP_CAT(Get, BOOST_PP_CAT(system, ()))))

class System
{
public:
  static const FactoryID factoryID = FactoryID::cSystem;

  virtual std::string GetName() = 0;

  virtual void Init() = 0;
  virtual void End() = 0;

};

#endif // !System_Guard
