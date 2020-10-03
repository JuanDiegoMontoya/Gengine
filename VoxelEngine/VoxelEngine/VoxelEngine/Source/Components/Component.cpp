/*HEADER_GOES_HERE*/

/* //// INCLUDES //// */
#include "../../Headers/Components/Component.h"
#include "../../Headers/Engine.h"
#include "../../Headers/Containers/Entity.h"
//#include "Graphics.h"
#include <Containers/Entity.h>


Space* Component::GetSpace() const
{
  return parent.space_;
}