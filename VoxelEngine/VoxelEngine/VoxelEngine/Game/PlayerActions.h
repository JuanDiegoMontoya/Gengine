#pragma once
#include <CoreEngine/ScriptableEntity.h>
#include <CoreEngine/Input.h>
#include <Voxels/VoxelManager.h>
#include <imgui/imgui.h>

class PlayerActions : public ScriptableEntity
{
public:
  PlayerActions(VoxelManager* vm) : voxels(vm) {}

  virtual void OnCreate() override
  {

  }

  virtual void OnDestroy() override
  {

  }

  virtual void OnUpdate(float dt) override
  {
    checkBlockPick();
    checkBlockDestruction();
    checkBlockPlacement();

    selected = (BlockType)glm::clamp((int)selected + (int)Input::GetScrollOffset().y, 0, (int)Block::PropertiesTable.size() - 1);
    
    ImGui::Begin("Held Block");
    ImGui::Text("%s", Block::PropertiesTable[(int)selected].name);
    ImGui::End();
  }


  void checkBlockPlacement()
  {
    if (Input::IsMousePressed(GLFW_MOUSE_BUTTON_2))
    {
      const auto cam = Camera::ActiveCamera;
      voxels->Raycast(
        cam->GetPos(),
        cam->GetFront(),
        5,
        std::function<bool(glm::vec3, Block, glm::vec3)>
        ([&](glm::vec3 pos, Block block, glm::vec3 side)->bool
          {
            if (block.GetType() == BlockType::bAir)
              return false;

            voxels->UpdateBlock(pos + side, selected);
            //chunkManager_.UpdateBlock(pos + side, selected, 0);
            
            return true;
          }
      ));
    }
  }


  void checkBlockDestruction()
  {
    if (Input::IsMousePressed(GLFW_MOUSE_BUTTON_1)/* &&
      !ImGui::IsAnyItemHovered() &&
      !ImGui::IsAnyItemActive() &&
      !ImGui::IsAnyItemFocused()*/)
    {
      const auto cam = Camera::ActiveCamera;
      voxels->Raycast(
        cam->GetPos(),
        cam->GetFront(),
        5,
        std::function<bool(glm::vec3, Block, glm::vec3)>
        ([&](glm::vec3 pos, Block block, glm::vec3 side)->bool
          {
            if (block.GetType() == BlockType::bAir)
              return false;

            voxels->UpdateBlock(pos, BlockType::bAir);
            //chunkManager_.UpdateBlock(pos, BlockType::bAir, 0);

            return true;
          }
      ));
    }
  }


  void checkBlockPick()
  {
    if (Input::IsMousePressed(GLFW_MOUSE_BUTTON_3) /*&&
      !ImGui::IsAnyItemHovered() &&
      !ImGui::IsAnyItemActive() &&
      !ImGui::IsAnyItemFocused()*/)
    {
      const auto cam = Camera::ActiveCamera;
      voxels->Raycast(
        cam->GetPos(),
        cam->GetFront(),
        5,
        std::function<bool(glm::vec3, Block, glm::vec3)>
        ([&](glm::vec3 pos, Block block, glm::vec3 side)->bool
          {
            if (block.GetType() == BlockType::bAir)
              return false;

            selected = block.GetType();

            return true;
          }
      ));
    }
  }

  BlockType selected = BlockType::bStone;
  VoxelManager* voxels{};
};