#pragma once
#include <CoreEngine/ScriptableEntity.h>
#include <CoreEngine/Input.h>
#include <Voxels/VoxelManager.h>
#include <imgui/imgui.h>
#include <Voxels/prefab.h>
#include <CoreEngine/Mesh.h>

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
    auto cam = GetComponent<Components::Parent>().entity.GetComponent<Components::Camera>();

    if (Input::IsKeyPressed(GLFW_KEY_V))
    {
      Entity ent = CreateEntity("Arrow");
      ent.AddComponent<Components::Transform>().SetTranslation(cam.GetWorldPos() + (cam.GetForward() * 1.f));
      ent.AddComponent<Components::BatchedMesh>().handle = MeshManager::GetMeshBatched("big_cube");
      ent.AddComponent<Components::Material>().handle = MaterialManager::GetMaterial("batchMaterial");
      auto collider = Physics::BoxCollider(glm::vec3(.5f));
      Components::DynamicPhysics phys(ent, Physics::MaterialType::TERRAIN, collider);
      ent.AddComponent<Components::DynamicPhysics>(std::move(phys)).Interface().AddForce(cam.GetForward() * 300.f);

      {
        Components::ParticleEmitter emitter(1500, "smoke.png");
        emitter.minLife = 1.0f;
        emitter.maxLife = 2.0f;
        emitter.interval = .001;
        emitter.minParticleOffset = { -.5, -.5, -.5 };
        emitter.maxParticleOffset = { .5, .5, .5 };
        emitter.minParticleAccel = { -.5, 1, -.5 };
        emitter.maxParticleAccel = { .5, 2, .5 };
        emitter.minParticleScale = { .3, .3 };
        emitter.maxParticleScale = { .3, .3 };
        //emitter.minParticleColor = { .75, .75, 0, .4 };
        //emitter.maxParticleColor = { 1, .75, 0, .8 };
        emitter.minParticleColor = { .75, .40, 0, .4 };
        emitter.maxParticleColor = { 1, .60, .2, .8 };
        ent.AddComponent<Components::ParticleEmitter>(std::move(emitter));
      }

      Entity child = CreateEntity("ArrowSmoke");
      child.SetParent(ent);
      child.AddComponent<Components::Transform>();
      child.AddComponent<Components::LocalTransform>();
      {
        Components::ParticleEmitter emitter2(3500, "smoke.png");
        emitter2.minLife = 3.0f;
        emitter2.maxLife = 4.0f;
        emitter2.interval = .001;
        emitter2.minParticleAccel = { -2, 1, -2 };
        emitter2.maxParticleAccel = { 2, 2, 2 };
        emitter2.minParticleScale = { .3, .3 };
        emitter2.maxParticleScale = { .3, .3 };
        emitter2.minParticleOffset = { -1, -1, -1 };
        emitter2.maxParticleOffset = { 1, 2, 1 };
        emitter2.minParticleColor = { .4, .4, .4, .4 };
        emitter2.maxParticleColor = { .5, .5, .5, .8 };
        child.AddComponent<Components::ParticleEmitter>(std::move(emitter2));
      }
    }

    checkTestButton();
    checkBlockPick();
    checkBlockDestruction(dt);
    checkBlockPlacement();

    selected = (BlockType)glm::clamp((int)selected + (int)Input::GetScrollOffset().y, 0, (int)Block::PropertiesTable.size() - 1);

    float size = 150.0f;
    ImGui::SetNextWindowBgAlpha(0.0f);
    ImGui::SetNextWindowSize(ImVec2(size * 1.25f, size * 1.7f));
    ImGui::SetNextWindowPos(ImVec2(32.0f, 1017 - size * 1.55f - 32.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::Begin("Held Block", 0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove);
    //ImGui::Begin("Held Block");
    ImGui::Text("Block Type:  %s\nPrefab Type: %s", Block::PropertiesTable[(int)selected].name, prefabName);
    ImGui::End();
    ImGui::PopStyleVar();

    ImGui::Begin("Yeet");
    ImGui::Text("Voxel raycast information:");
    float dist = 5.f;
    ImGui::Text("Ray length: %0.f", dist);
    //const auto cam = Camera::ActiveCamera;
    voxels->Raycast(
      CameraSystem::GetPos(),
      CameraSystem::GetFront(),
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
    if (Input::IsInputActionPressed("Build"))
    {
      //const auto cam = CameraSystem::ActiveCamera;
      voxels->Raycast(
         CameraSystem::GetPos(),
         CameraSystem::GetFront(),
        5,
        std::function<bool(glm::vec3, Block, glm::vec3)>
        ([&](glm::vec3 pos, Block block, glm::vec3 side)->bool
          {
            if (block.GetType() == BlockType::bAir)
              return false;

            if (Input::IsKeyDown(GLFW_KEY_H) || Input::IsKeyDown(GLFW_KEY_J) || Input::IsKeyDown(GLFW_KEY_K) || Input::IsKeyDown(GLFW_KEY_L))
            {
              if (Input::IsKeyDown(GLFW_KEY_J))
              {
                prefab = PrefabManager::GetPrefab("OakTree");
                prefabName = "OakTree";
              }
              else if (Input::IsKeyDown(GLFW_KEY_K))
              {
                prefab = PrefabManager::GetPrefab("OakTreeBig");
                prefabName = "OakTreeBig";
              }
              if (Input::IsKeyDown(GLFW_KEY_L))
              {
                prefab = PrefabManager::GetPrefab("Error");
                prefabName = "Error";
              }

              bool spawn = true;

              if (prefab.GetPlacementType() == PlacementType::PriorityRequired)
              {
                for (unsigned i = 0; i < prefab.blocks.size(); i++)
                {
                  if (voxels->GetBlock((glm::ivec3)(pos + side) + prefab.blocks[i].first).GetType() != BlockType::bAir)
                  {
                    spawn = false;
                    break;
                  }
                }
              }

              if (spawn)
              {
                for (unsigned i = 0; i < prefab.blocks.size(); i++)
                {
                  if (prefab.GetPlacementType() != PlacementType::NoOverwriting || voxels->GetBlock((glm::ivec3)(pos + side) + prefab.blocks[i].first).GetType() == BlockType::bAir)
                  {
                    if (prefab.blocks[i].second.GetPriority() >= voxels->GetBlock((glm::ivec3)(pos + side) + prefab.blocks[i].first).GetPriority())
                      voxels->UpdateBlock((glm::ivec3)(pos + side) + prefab.blocks[i].first, prefab.blocks[i].second.GetType());
                  }
                }
              }
            }
            else
            {
              voxels->UpdateBlock(pos + side, selected);
            }
            //chunkManager_.UpdateBlock(pos + side, selected, 0);

            return true;
          }
      ));
    }
  }


  void checkBlockDestruction(float dt)
  {
    if (Input::GetInputAxis("Attack")
      /*if (Input::IsMousePressed(GLFW_MOUSE_BUTTON_1) &&
        !ImGui::IsAnyItemHovered() &&
        !ImGui::IsAnyItemActive() &&
        !ImGui::IsAnyItemFocused()*/)
    {
      bool hit = false;
      //const auto cam = CameraSystem::ActiveCamera;
      voxels->Raycast(
         CameraSystem::GetPos(),
         CameraSystem::GetFront(),
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
    
    if (Input::IsMousePressed(GLFW_MOUSE_BUTTON_MIDDLE) /*&&
      !ImGui::IsAnyItemHovered() &&
      !ImGui::IsAnyItemActive() &&
      !ImGui::IsAnyItemFocused()*/)
    {
      //const auto cam = CameraSystem::ActiveCamera;
      voxels->Raycast(
         CameraSystem::GetPos(),
         CameraSystem::GetFront(),
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

  glm::vec3 prevBlock = { -1, -1, -1 };
  bool prevHit = false;
  float timer = 0.0f;
  BlockType selected = BlockType::bStone;
  VoxelManager* voxels{};
  Prefab prefab = PrefabManager::GetPrefab("Error");
  std::string prefabName = "Error";
};