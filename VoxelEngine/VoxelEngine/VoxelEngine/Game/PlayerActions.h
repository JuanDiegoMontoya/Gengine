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
    checkTestButton();
    checkBlockPick();
    checkBlockDestruction();
    checkBlockPlacement();

    selected = (BlockType)glm::clamp((int)selected + (int)Input::GetScrollOffset().y, 0, (int)Block::PropertiesTable.size() - 1);
    
    float size = 100.0f;
    ImGui::SetNextWindowBgAlpha(0.0f);
    ImGui::SetNextWindowSize(ImVec2(size * 1.25f, size * 1.7f));
    ImGui::SetNextWindowPos(ImVec2(32.0f, 1017 - size * 1.55f - 32.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::Begin("Held Block", 0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove);
    //ImGui::Begin("Held Block");
    ImGui::Text("%s", Block::PropertiesTable[(int)selected].name);
    ImGui::End();
    ImGui::PopStyleVar();
  }


  void checkTestButton()
  {
    if (Input::IsKeyDown(GLFW_KEY_G))
    {
      for (int i = 0; i < 100; i++)
      {
        //voxels->MeshChunk({ rand() % 5, rand() % 5, rand() % 5 });
        voxels->UpdateBlock({ rand() % 100, rand() % 100, rand() % 100 }, Block(BlockType::bStone));
      }
    }
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