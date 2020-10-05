                                           #ifndef FactoryID_Guard                                          
#define FactoryID_Guard                                          
                                                                 
#include <memory>                                                
                                                                 
#include "PreProcessorMagic.h"                                 
#include "Containers/Properties.h"                             
                                                                 
                                                                 
enum class FactoryID                                             
  {                                                              
    cEngine,                                                     
    cManager,                                                    
    cSystem,                                                     
    cSpace,                                                      
    cObject,                                                     
    cComponent,                                                  
    cEvent,                                                      
    cCount                                                       
  };                                                             
                                                                 
                                                                 
//ManagerIDs
const ID cFrameRateController = 0;
const ID cGraphicsManager = 1;
const ID cInputManager = 2;
const ID cStubManager = 3;
const ID cTraceManager = 4;

//SystemIDs
const ID cCamera = 0;
const ID cStubSystem = 1;
const ID cTestingSystem = 2;
const ID cVoxelWorld = 3;

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
  