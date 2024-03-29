#pragma once
#include <glm/glm.hpp>
#include <engine/gfx/RenderView.h>
#include <span>

namespace Voxels
{
  class VoxelManager;
}

class Scene;

class Editor
{
public:
  Editor(Voxels::VoxelManager& vm) : voxels(vm) {}
  void Update(Scene* scene);

private:
  void SaveRegion();
  void LoadRegion();
  void CancelSelection();
  void SelectBlock();
  void DrawSelection(std::span<GFX::RenderView*> renderViews);

  Voxels::VoxelManager& voxels;

  const int pickLength = 5;// ray cast distance
  int selectedPositions{};  // how many positions have been selected
  glm::vec3 wpositions[3]{};  // selected positions (0-3)
  glm::vec3 hposition{};      // hovered position (others are locked)
  bool open = false;
  char sName[256]{};
  char lName[256]{};
  bool skipAir = false; // if true, newly saved prefabs will skip air blocks within
};