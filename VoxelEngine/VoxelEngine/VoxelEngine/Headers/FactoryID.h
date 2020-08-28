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
                                                                 
                                                                 
//SystemIDs
const ID cFrameRateController = 0;
const ID cStubSystem = 1;
const ID cTraceSystem = 2;

//ComponentIDs
const ID cStubComponent = 0;
const ID cTestingComponent = 1;

//EventIDs
const ID cDestroyEvent = 0;
const ID cDrawEvent = 1;
const ID cInitEvent = 2;
const ID cStubEvent = 3;
const ID cTestingEvent = 4;
const ID cTraceEvent = 5;
const ID cUpdateEvent = 6;

#endif // !FactoryID_Guard                                       
  