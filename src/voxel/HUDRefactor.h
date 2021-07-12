#pragma once
#include <voxel/block.h>

// renders stuff directly to the screen
class HUD
{
public:
  void Update();

  Voxels::BlockType GetSelected() { return selected_; }
  void SetSelected(Voxels::BlockType s) { selected_ = s; }
private:
  Voxels::BlockType selected_ = Voxels::BlockType::bError;
};