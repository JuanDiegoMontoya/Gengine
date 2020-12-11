#ifndef VoxelWorld_Guard
#define VoxelWorld_Guard

#include <iostream>
#include "../Containers/Properties.h"
#include "System.h"

class UpdateEvent;
class DrawEvent;
class ChunkManager;
class HUD;

class VoxelWorld : public System
{
public:
  static const ID systemType = cVoxelWorld;

  VoxelWorld();
  ~VoxelWorld();
  void Init();
  void End();

  std::unique_ptr<System> Clone() const;
  std::string GetName();

  void UpdateEventsListen(UpdateEvent* updateEvent);
  void DrawEventsListen(DrawEvent* drawEvent);

  static std::unique_ptr<VoxelWorld> RegisterVoxelWorld();

  struct Settings
  {
    bool gammaCorrection = true;
    float fogStart = 500.f;
    float fogEnd = 3000.f;
  }static inline settings;
private:

  friend void RegisterSystems();

  // TODO: HUD is TEMPORARY
  std::unique_ptr<ChunkManager> chunkManager_;
  std::unique_ptr<HUD> hud_;
};

#endif // !VoxelWorld_Guard