#include "gPCH.h"
#include <game/PlayerActions.h>
#include <imgui/imgui.h>
#include <engine/gfx/Mesh.h>
#include <engine/Input.h>
#include <game/FlyingPlayerController.h>
#include <game/KinematicPlayerController.h>
#include <engine/Engine.h>
#include <engine/ecs/system/ParticleSystem.h>
#include <engine/gfx/TextureManager.h>
#include <engine/gfx/RenderView.h>
#include <engine/gfx/Camera.h>

#include <engine/ecs/component/ParticleEmitter.h>
#include <engine/ecs/component/Physics.h>
#include <engine/ecs/component/Scripting.h>
#include <engine/ecs/component/Transform.h>
#include <engine/ecs/component/Rendering.h>
#include <engine/ecs/component/Core.h>

#include <glm/gtc/type_ptr.hpp>

#include "KinematicPlayerController.h"

using namespace Voxels;

void PlayerActions::OnCreate()
{

}

void PlayerActions::OnDestroy()
{
}

void PlayerActions::OnUpdate(Timestep timestep)
{
  if (Input::IsKeyPressed(GLFW_KEY_P))
  {
    GetScene()->GetEngine()->IsPaused() ? GetScene()->GetEngine()->Unpause() : GetScene()->GetEngine()->Pause();
  }

  Entity parent = GetComponent<Component::Parent>().entity;
  auto* camera = GetScene()->GetRenderView("main").camera;
  auto& vi = camera->viewInfo;
  auto* controller = dynamic_cast<KinematicPlayerController*>(parent.GetComponent<Component::NativeScriptComponent>().Instance);

  if (Input::IsKeyPressed(GLFW_KEY_V))
  {
    std::optional<Entity> arrow = GetScene()->GetEntity("Arrow");
    if (arrow)
    {
      //arrow->Destroy();
    }

    Entity ent = CreateEntity("Arrow");
    ent.AddComponent<Component::Transform>().SetTranslation(vi.position + (vi.GetForwardDir() * 1.5f));
    ent.AddComponent<Component::Model>();
    ent.AddComponent<Component::BatchedMesh>().handle = MeshManager::GetMeshBatched("big_cube");
    ent.AddComponent<Component::Material>().handle = GFX::MaterialManager::Get()->GetMaterial("batchMaterial");
    ent.AddComponent<Component::InterpolatedPhysics>();
    auto collider = Physics::BoxCollider(glm::vec3(.5f));
    Component::DynamicPhysics phys(ent, Physics::MaterialType::TERRAIN, collider);
    auto& physics = ent.AddComponent<Component::DynamicPhysics>(std::move(phys));
    physics.Interface().SetVelocity(controller->velocity);
    physics.Interface().AddForce(vi.GetForwardDir() * 300.f);
    auto& lifetime = ent.AddComponent<Component::Lifetime>();
    lifetime.active = true;
    lifetime.remainingSeconds = 2;

    {
      Component::ParticleEmitter emitter{};
      emitter.handle = ParticleManager::Get().MakeParticleEmitter(1500, "smoke");
      emitter.data.minLife = 1.0f;
      emitter.data.maxLife = 2.0f;
      emitter.data.interval = .001;
      emitter.data.minParticleOffset = { -.5, -.5, -.5 };
      emitter.data.maxParticleOffset = { .5, .5, .5 };
      emitter.data.minParticleAccel = { -.5, 1, -.5 };
      emitter.data.maxParticleAccel = { .5, 2, .5 };
      emitter.data.minParticleScale = { .3, .3 };
      emitter.data.maxParticleScale = { .3, .3 };
      //emitter.minParticleColor = { .75, .75, 0, .4 };
      //emitter.maxParticleColor = { 1, .75, 0, .8 };
      emitter.data.minParticleColor = { .75, .40, 0, .1 };
      emitter.data.maxParticleColor = { 1, .60, .2, .2 };
      ent.AddComponent<Component::ParticleEmitter>(std::move(emitter));
    }

    Entity child = CreateEntity("ArrowSmoke");
    child.SetParent(ent);
    child.AddComponent<Component::Transform>();
    child.AddComponent<Component::LocalTransform>();
    {
      Component::ParticleEmitter emitter2{};
      emitter2.handle = ParticleManager::Get().MakeParticleEmitter(3500, "smoke");
      emitter2.data.minLife = 3.0f;
      emitter2.data.maxLife = 4.0f;
      emitter2.data.interval = .001;
      emitter2.data.minParticleAccel = { -2, 1, -2 };
      emitter2.data.maxParticleAccel = { 2, 2, 2 };
      emitter2.data.minParticleScale = { .3, .3 };
      emitter2.data.maxParticleScale = { .3, .3 };
      emitter2.data.minParticleOffset = { -1, -1, -1 };
      emitter2.data.maxParticleOffset = { 1, 2, 1 };
      emitter2.data.minParticleColor = { .4, .4, .4, .1 };
      emitter2.data.maxParticleColor = { .5, .5, .5, .2 };
      child.AddComponent<Component::ParticleEmitter>(std::move(emitter2));
    }
  }

  ImGui::Begin("Particle Emitter Settings");
  if (ImGui::Button("Spawn Emitter"))
  {
    Entity ent = CreateEntity("SpawnedEmitter");
    ent.AddComponent<Component::Transform>().SetTranslation(vi.position + (vi.GetForwardDir() * 1.5f));
    ent.AddComponent<Component::BatchedMesh>().handle = MeshManager::GetMeshBatched("big_cube");
    ent.AddComponent<Component::Material>().handle = GFX::MaterialManager::Get()->GetMaterial("batchMaterial");
    ent.AddComponent<Component::Model>();
    auto collider = Physics::BoxCollider(glm::vec3(.5f));
    Component::DynamicPhysics phys(ent, Physics::MaterialType::TERRAIN, collider);
    ent.AddComponent<Component::DynamicPhysics>(std::move(phys));

    Component::ParticleEmitter emitter{};
    emitter.handle = ParticleManager::Get().MakeParticleEmitter(maxParticles, "smoke");
    emitter.data.minLife = minLife;
    emitter.data.maxLife = maxLife;
    emitter.data.interval = interval;
    emitter.data.minParticleOffset = minParticleOffset;
    emitter.data.maxParticleOffset = maxParticleOffset;
    emitter.data.minParticleVelocity = minParticleVelocity;
    emitter.data.maxParticleVelocity = maxParticleVelocity;
    emitter.data.minParticleAccel = minParticleAccel;
    emitter.data.maxParticleAccel = maxParticleAccel;
    emitter.data.minParticleScale = minParticleScale;
    emitter.data.maxParticleScale = maxParticleScale;
    emitter.data.minParticleColor = minParticleColor;
    emitter.data.maxParticleColor = maxParticleColor;
    ent.AddComponent<Component::ParticleEmitter>(std::move(emitter));
  }
  ImGui::DragInt("Max particles", &maxParticles, 100, 0, 1'000'000);
  if (ImGui::Button("Compute interval"))
  {
    interval = ((maxLife + minLife) / 2.0f) / maxParticles;
  }
  ImGui::DragFloat("Interval", &interval, .003f, 0.00000001f, 1.f, "%.8f", ImGuiSliderFlags_Logarithmic);
  ImGui::DragFloat4("Min color", glm::value_ptr(minParticleColor), 0.01f, 0.f, 1.f);
  ImGui::DragFloat4("Max color", glm::value_ptr(maxParticleColor), 0.01f, 0.f, 1.f);
  ImGui::DragFloat("Min life", &minLife, .05f, 0.f, 5.f);
  ImGui::DragFloat("Max life", &maxLife, .05f, 0.f, 5.f);
  ImGui::DragFloat3("Min pos", glm::value_ptr(minParticleOffset), .03f, -5.f, 5.f);
  ImGui::DragFloat3("Max pos", glm::value_ptr(maxParticleOffset), .03f, -5.f, 5.f);
  ImGui::DragFloat3("Min vel", glm::value_ptr(minParticleVelocity), .03f, -5.f, 5.f);
  ImGui::DragFloat3("Max vel", glm::value_ptr(maxParticleVelocity), .03f, -5.f, 5.f);
  ImGui::DragFloat3("Min accel", glm::value_ptr(minParticleAccel), .03f, -5.f, 5.f);
  ImGui::DragFloat3("Max accel", glm::value_ptr(maxParticleAccel), .03f, -5.f, 5.f);
  ImGui::DragFloat2("Min scale", glm::value_ptr(minParticleScale), .025f, 0.f, 5.f);
  ImGui::DragFloat2("Max scale", glm::value_ptr(maxParticleScale), .025f, 0.f, 5.f);

  std::optional<Entity> rain = GetScene()->GetEntity("rainmaker");
  auto& meme = rain->GetComponent<Component::ParticleEmitter>();
  ImGui::DragFloat3("Min pos2", glm::value_ptr(meme.data.minParticleOffset), .03f, -5.f, 5.f);
  ImGui::DragFloat3("Max pos2", glm::value_ptr(meme.data.maxParticleOffset), .03f, -5.f, 5.f);
  ImGui::End();

  checkTestButton();
  checkBlockPick();
  checkBlockDestruction(timestep.dt_effective);
  checkBlockPlacement();

  selected = (BlockType)glm::clamp((int)selected + (int)Input::GetScrollOffset().y, 0, (int)Block::PropertiesTable.size() - 1);

  float size = 150.0f;
  ImGui::SetNextWindowBgAlpha(0.0f);
  ImGui::SetNextWindowSize(ImVec2(size * 2.25f, size * 1.7f));
  ImGui::SetNextWindowPos(ImVec2(32.0f, 1017 - size * 1.55f - 32.0f));
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
  ImGui::Begin("Held Block", 0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove);
  //ImGui::Begin("Held Block");
  ImGui::Text("Block Type:  %s\nPrefab Type: %s", Block::PropertiesTable[(int)selected].name, prefabName.c_str());
  ImGui::End();
  ImGui::PopStyleVar();

  ImGui::Begin("Yeet");
  ImGui::Text("Voxel raycast information:");
  float dist = 5.f;
  ImGui::Text("Ray length: %0.f", dist);
  //const auto cam = Camera::ActiveCamera;
  voxels->Raycast(
    vi.position,
    vi.GetForwardDir(),
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
  //if (ImGui::Button("Toggle Flying"))
  //{
  //  fly = !fly;
  //  this->GetComponent<Components::NativeScriptComponent>().DestroyScript();
  //  if (fly)
  //  {
  //    this->GetComponent<Components::NativeScriptComponent>().Bind<FlyingPlayerController>();
  //  }
  //  else
  //  {
  //    this->GetComponent<Components::NativeScriptComponent>().Bind<KinematicPlayerController>();
  //  }
  //}
  ImGui::End();
}

void PlayerActions::checkTestButton()
{
  if (Input::IsKeyDown(GLFW_KEY_G))
  {
    auto chunks = voxels->GetChunksRegionWorldSpace(glm::ivec3(0, 30, 0) + Voxels::Chunk::CHUNK_SIZE, glm::ivec3(49, 49, 49) + Voxels::Chunk::CHUNK_SIZE);
    for (auto chunk : chunks) chunk->Lock();
    for (int i = 0; i < 100; i++)
    {
      //voxels->MeshChunk({ rand() % 5, rand() % 5, rand() % 5 });
      voxels->UpdateBlockCheap(glm::ivec3(rand() % 50, rand() % 20 + 30, rand() % 50) + Voxels::Chunk::CHUNK_SIZE, Block(BlockType::bStone));
    }
    for (auto chunk : chunks)
    {
      voxels->UpdateChunk(chunk);
      chunk->Unlock();
    }
  }
}

void PlayerActions::checkBlockPlacement()
{
  if (Input::IsInputActionPressed("Build"))
  {
    auto& vi = GetScene()->GetRenderView("main").camera->viewInfo;

    //const auto cam = CameraSystem::ActiveCamera;
    voxels->Raycast(
      vi.position,
      vi.GetForwardDir(),
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

void PlayerActions::checkBlockDestruction(float dt)
{
  if (Input::GetInputAxis("Attack")
    /*if (Input::IsMousePressed(GLFW_MOUSE_BUTTON_1) &&
      !ImGui::IsAnyItemHovered() &&
      !ImGui::IsAnyItemActive() &&
      !ImGui::IsAnyItemFocused()*/)
  {
    bool hit = false;
    auto& vi = GetScene()->GetRenderView("main").camera->viewInfo;
    //const auto cam = CameraSystem::ActiveCamera;
    voxels->Raycast(
      vi.position,
      vi.GetForwardDir(),
      5,
      std::function<bool(glm::vec3, Block, glm::vec3)>
      ([&](glm::vec3 pos, Block block, [[maybe_unused]] glm::vec3 side)->bool
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

void PlayerActions::checkBlockPick()
{

  if (Input::IsMousePressed(GLFW_MOUSE_BUTTON_MIDDLE) /*&&
    !ImGui::IsAnyItemHovered() &&
    !ImGui::IsAnyItemActive() &&
    !ImGui::IsAnyItemFocused()*/)
  {
    auto& vi = GetScene()->GetRenderView("main").camera->viewInfo;
    //const auto cam = CameraSystem::ActiveCamera;
    voxels->Raycast(
      vi.position,
      vi.GetForwardDir(),
      5,
      std::function<bool(glm::vec3, Block, glm::vec3)>
      ([&]([[maybe_unused]] glm::vec3 pos, Block block, [[maybe_unused]] glm::vec3 side)->bool
        {
          if (block.GetType() == BlockType::bAir)
            return false;

          selected = block.GetType();

          return true;
        }
    ));
  }
}
