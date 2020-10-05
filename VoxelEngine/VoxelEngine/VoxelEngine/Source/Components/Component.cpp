/*HEADER_GOES_HERE*/

/* //// INCLUDES //// */
#include "../../Headers/Components/Component.h"
#include "../../Headers/Engine.h"
#include "../../Headers/Containers/Object.h"
//#include "Graphics.h"


Space* Component::GetSpace() const
{
  return parent->GetSpace();
}