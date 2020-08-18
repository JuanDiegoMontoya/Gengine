/*HEADER_GOES_HERE*/
#ifndef FactoryID_Guard
#define FactoryID_Guard

#include <memory>

#include "PreProcessorMagic.h"
#include "Containers/Properties.h"


enum class FactoryID 
{
  cEngine,
  cSystem,
  cSpace,
  cObject,
  cComponent,
  cEvent,
  cCount
};

#endif // !FactoryID_Guard
