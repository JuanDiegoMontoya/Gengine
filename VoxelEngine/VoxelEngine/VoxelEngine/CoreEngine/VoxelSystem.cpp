/*HEADER_GOES_HERE*/
#include "VoxelSystem.h"

#include <Refactor/hud.h>
#include <World/chunk_manager.h>
#include <WorldGen/WorldGen2.h>
#include <World/chunk_manager.h>
#include <Rendering/ChunkRenderer.h>
#include <Input.h>

#include <Engine.h>
#include <Chunks/ChunkSerialize.h>
#include <Chunks/ChunkStorage.h>

VoxelSystem::VoxelSystem()
{
  chunkManager_ = std::make_unique<ChunkManager>();
  hud_ = std::make_unique<HUD>();
}

VoxelSystem::~VoxelSystem()
{
}

void VoxelSystem::Init()
{
  WorldGen2::Init();
  chunkManager_->Init();
  WorldGen2::GenerateWorld();
  ChunkRenderer::InitAllocator();
  chunkManager_->initializeSunlight();
  WorldGen2::InitMeshes();
  WorldGen2::InitBuffers();

  NuRenderer::Init();
  
  auto compressed = CompressChunk(ChunkStorage::GetChunk(glm::ivec3(0))->GetStorage());
}


void VoxelSystem::Update(float dt)
{
  // update each camera
  //if (!Interface::activeCursor)
  //{
  //  GetCurrentCamera()->Update(Engine::GetDT());
  //}

  //if (doCollisionTick)
  //{
  //  CheckCollision();
  //}

  chunkManager_->Update();
  //CheckInteraction();
  //sun_->Update();
  //Renderer::DrawAll();
  //Editor::Update();
  //hud_.Update();
  //DrawImGui();
}

// TODO: move this code into the system or sumthin
void VoxelSystem::Draw()
{
  NuRenderer::DrawAll();
  //glfwSwapBuffers(GraphicsManager::GetGraphicsManager()->GetWindow());
}