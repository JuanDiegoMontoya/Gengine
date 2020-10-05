#ifndef MANAGER_COUNT
#define MANAGER_COUNT 5
#endif

#ifdef ENGINE_RUNNING
#include "FrameRateController.h"
#include "GraphicsManager.h"
#include "InputManager.h"
#include "StubManager.h"
#include "TraceManager.h"
#endif // !ENGINE_RUNNING

#ifdef FACTORY_RUNNING

#include "../Headers/Factory.h"
#include "FrameRateController.h"
#include "GraphicsManager.h"
#include "InputManager.h"
#include "StubManager.h"
#include "TraceManager.h"


void RegisterManagers()
{
Factory::ManagerPropertyMap["FrameRateController"] = std::vector<PropertyID>({
  PropertyID("locked", FrameRateController::locked_id, offsetof(FrameRateController, locked), sizeof(Bool)),
  PropertyID("fps", FrameRateController::fps_id, offsetof(FrameRateController, fps), sizeof(Int)),
  });
Factory::ManagerPropertyMap["GraphicsManager"] = std::vector<PropertyID>({
  });
Factory::ManagerPropertyMap["InputManager"] = std::vector<PropertyID>({
  });
Factory::ManagerPropertyMap["StubManager"] = std::vector<PropertyID>({
  });
Factory::ManagerPropertyMap["TraceManager"] = std::vector<PropertyID>({
  });
}

#endif // !FACTORY_RUNNING