/*HEADER_GOES_HERE*/

/* //// INCLUDES //// */
#include "../../Headers/Systems/System.h"
#include "../../Headers/Engine.h"
#include "../../Headers/Containers/Object.h"
//#include "Graphics.h"


Space* System::GetSpace() const
{
  return parent->GetSpace();
}