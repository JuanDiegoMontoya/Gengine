#ifndef AllComponentHeaders_Guard
#define AllComponentHeaders_Guard

#define COMPONENT_COUNT 1

#ifdef FACTORY_RUNNING

#include "../Headers/Factory.h"
#include "StubComponent.h"


void RegisterComponents()
{
Factory::ComponentPropertyMap["StubComponent"] = std::vector<PropertyID>({
  });
}

inline std::unique_ptr<Component> Component0() { return std::make_unique<StubComponent>(StubComponent(); }


#endif // !FACTORY_RUNNING

#endif // !AllComponentHeaders_Guard