#pragma once
#include <memory>

class UpdateEvent;
class DrawEvent;
class ChunkManager;
class HUD;

class VoxelWorld
{
public:
  VoxelWorld();
  ~VoxelWorld();
  void Init();
  void End();

  void Update(float dt);
  void Draw();

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