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
    checkBlockDestruction(dt);
    checkBlockPlacement();

    printf("%.3f ", timer);

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

    ImGui::Begin("Yeet");
    ImGui::Text("Voxel raycast information:");
    float dist = 5.f;
    ImGui::Text("Ray length: %0.f", dist);
    const auto cam = Camera::ActiveCamera;
    voxels->Raycast(
      cam->GetPos(),
      cam->GetFront(),
      dist,
      [this](glm::vec3 pos, Block block, glm::vec3 side)->bool
      {
        if (block.GetType() == BlockType::bAir)
        {
          return false;
        }

        ImGui::Text("Block Type: %d (%s)", block.GetTypei(), block.GetName());
        //ImGui::Text("Write Strength: %d", block->WriteStrength());
        //ImGui::Text("Light Value: %d", block->LightValue());
        Light lit;
        if (auto opt = voxels->TryGetBlock(pos))
          lit = opt->GetLight();
        Light lit2;
        if (auto opt = voxels->TryGetBlock(pos + side))
          lit2 = opt->GetLight();
        ImGui::Text("Light:  (%d, %d, %d, %d)", lit.GetR(), lit.GetG(), lit.GetB(), lit.GetS());
        ImGui::Text("FLight: (%d, %d, %d, %d)", lit2.GetR(), lit2.GetG(), lit2.GetB(), lit2.GetS());
        ImGui::Text("Block pos:  (%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z);
        ImGui::Text("Block side: (%.2f, %.2f, %.2f)", side.x, side.y, side.z);

        return true;
      });
    ImGui::End();
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


  void checkBlockDestruction(float dt)
  {
    if(Input::IsMouseDown(GLFW_MOUSE_BUTTON_1)
    /*if (Input::IsMousePressed(GLFW_MOUSE_BUTTON_1) &&
      !ImGui::IsAnyItemHovered() &&
      !ImGui::IsAnyItemActive() &&
      !ImGui::IsAnyItemFocused()*/)
    {
      bool hit = false;
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

            hit = true;
            timer += dt;
            if (pos == prevBlock && prevHit && timer >= block.GetTTK() && block.GetDestructible())
            {
              voxels->UpdateBlock(pos, BlockType::bAir);
              timer = 0.0f;
              hit = false;
            }
            else
            {
              if (prevBlock != pos)
              {
                timer = dt;
              }
              prevBlock = pos;
              if (!block.GetDestructible())
                timer = 0.0f;
            }

            //chunkManager_.UpdateBlock(pos, BlockType::bAir, 0);

            return true;
          }
      ));
      prevHit = hit;
      if (!prevHit)
        timer = 0.0f;
    }
    else
    {
      prevHit = false;
      timer = 0.0f;
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

  glm::vec3 prevBlock = {-1, -1, -1};
  bool prevHit = false;
  float timer = 0.0f;
  BlockType selected = BlockType::bStone;
  VoxelManager* voxels{};
};