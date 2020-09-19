/*HEADER_GOES_HERE*/
#include "../../Headers/Components/VoxelWorld.h"
#include "../../Headers/Containers/Object.h"
#include "../../Headers/Containers/Space.h"
#include "../../Headers/Factory.h"

#include <Events/UpdateEvent.h>
#include <Events/DrawEvent.h>

#include <Systems/GraphicsSystem.h>
#include <Refactor/hud.h>
#include <World/chunk_manager.h>
#include <WorldGen/WorldGen2.h>
#include <World/chunk_manager.h>
#include <Rendering/ChunkRenderer.h>
#include <Systems/InputSystem.h>
#include <Rendering/Renderer.h>


std::string VoxelWorld::GetName() { return "VoxelWorld"; }

std::unique_ptr<VoxelWorld> VoxelWorld::RegisterVoxelWorld() 
{
  return std::make_unique<VoxelWorld>();
}

VoxelWorld::VoxelWorld()
  : Component(componentType)
{
  chunkManager_ = std::make_unique<ChunkManager>();
}

VoxelWorld::~VoxelWorld()
{
}

void VoxelWorld::Init()
{
  GetSpace()->RegisterListener(this, &VoxelWorld::UpdateEventsListen);
  GetSpace()->RegisterListener(this, &VoxelWorld::DrawEventsListen);

  WorldGen2::Init();
  chunkManager_->Init();
  WorldGen2::GenerateWorld();
  ChunkRenderer::InitAllocator();
  chunkManager_->initializeSunlight();
  WorldGen2::InitMeshes();
  WorldGen2::InitBuffers();

  NuRenderer::Init();
}

void VoxelWorld::End()
{
  if (parent != nullptr)
  {
    GetSpace()->UnregisterListener(this, &VoxelWorld::UpdateEventsListen);
    GetSpace()->UnregisterListener(this, &VoxelWorld::DrawEventsListen);
  }
}

std::unique_ptr<Component> VoxelWorld::Clone() const
{
  auto result = new VoxelWorld();
  assert(false); // do not copy this shit
  return std::unique_ptr<Component>(result);
}

void VoxelWorld::UpdateEventsListen(UpdateEvent* updateEvent)
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
void VoxelWorld::DrawEventsListen(DrawEvent* drawEvent)
{
  NuRenderer::DrawAll();
}