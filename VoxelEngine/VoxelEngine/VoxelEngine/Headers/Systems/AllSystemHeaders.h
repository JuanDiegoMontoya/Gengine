#ifndef SYSTEM_COUNT
#define SYSTEM_COUNT 6
#endif

#ifdef ENGINE_RUNNING
#include "FrameRateController.h"
#include "GraphicsSystem.h"
#include "InputSystem.h"
#include "StubSystem.h"
#include "TraceSystem.h"
#include "VoxelWorld.h"
#endif // !ENGINE_RUNNING

#ifdef FACTORY_RUNNING

#include "../Headers/Factory.h"
#include "FrameRateController.h"
#include "GraphicsSystem.h"
#include "InputSystem.h"
#include "StubSystem.h"
#include "TraceSystem.h"
#include "VoxelWorld.h"


void RegisterSystems()
{
Factory::SystemPropertyMap["FrameRateController"] = std::vector<PropertyID>({
  PropertyID("locked", FrameRateController::locked_id, offsetof(FrameRateController, locked), sizeof(Bool)),
  PropertyID("fps", FrameRateController::fps_id, offsetof(FrameRateController, fps), sizeof(Int)),
  });
Factory::SystemPropertyMap["GraphicsSystem"] = std::vector<PropertyID>({
  });
Factory::SystemPropertyMap["InputSystem"] = std::vector<PropertyID>({
  });
Factory::SystemPropertyMap["StubSystem"] = std::vector<PropertyID>({
  });
Factory::SystemPropertyMap["TraceSystem"] = std::vector<PropertyID>({
  });
Factory::SystemPropertyMap["VoxelWorld"] = std::vector<PropertyID>({
  });
}

#endif // !FACTORY_RUNNING