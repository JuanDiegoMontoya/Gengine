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

//ComponentIDs
const ID cStubComponent = 0;
const ID cTestingComponent = 1;

//EventIDs
const ID cDrawEvent = 0;
const ID cStubEvent = 1;
const ID cUpdateEvent = 2;

#endif // !FactoryID_Guard                                       
  