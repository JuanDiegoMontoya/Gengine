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
const ID cGraphicsSystem = 1;
const ID cInputSystem = 2;
const ID cStubSystem = 3;
const ID cTraceSystem = 4;
const ID cVoxelWorld = 5;

//ComponentIDs
const ID cCamera = 0;
const ID cStubComponent = 1;
const ID cTag = 2;
const ID cTestingComponent = 3;
const ID cVoxelWorld = 4;

//EventIDs
const ID cDestroyEvent = 0;
const ID cDrawEvent = 1;
const ID cInitEvent = 2;
const ID cRenderEvent = 3;
const ID cStubEvent = 4;
const ID cTestingEvent = 5;
const ID cTraceEvent = 6;
const ID cUpdateEvent = 7;

#endif // !FactoryID_Guard                                       
  