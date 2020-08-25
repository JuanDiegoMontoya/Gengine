#ifndef SYSTEM_COUNT
#define SYSTEM_COUNT 3
#endif

#ifdef ENGINE_RUNNING
#include "FrameRateController.h"
#include "StubSystem.h"
#include "TraceSystem.h"
#endif // !ENGINE_RUNNING

#ifdef FACTORY_RUNNING

#include "../Headers/Factory.h"
#include "FrameRateController.h"
#include "StubSystem.h"
#include "TraceSystem.h"


void RegisterSystems()
{
Factory::SystemPropertyMap["FrameRateController"] = std::vector<PropertyID>({
  PropertyID("locked", FrameRateController::locked_id, offsetof(FrameRateController, locked), sizeof(Bool)),
  PropertyID("fps", FrameRateController::fps_id, offsetof(FrameRateController, fps), sizeof(Int)),
  });
Factory::SystemPropertyMap["StubSystem"] = std::vector<PropertyID>({
  });
Factory::SystemPropertyMap["TraceSystem"] = std::vector<PropertyID>({
  });
}

#endif // !FACTORY_RUNNING