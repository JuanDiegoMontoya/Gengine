/*HEADER_GOES_HERE*/
#include "../../Headers/Systems/VoxelWorld.h"
#include "../../Headers/Containers/Object.h"
#include "../../Headers/Containers/Space.h"
#include "../../Headers/Factory.h"

#include <Events/UpdateEvent.h>
#include <Events/DrawEvent.h>
#include <Events/RenderEvent.h>

#include <Managers/GraphicsManager.h>
#include <Refactor/hud.h>
#include <World/chunk_manager.h>
#include <WorldGen/WorldGen2.h>
#include <World/chunk_manager.h>
#include <Rendering/ChunkRenderer.h>
#include <Managers/InputManager.h>

#include <Engine.h>
#include <Chunks/ChunkSerialize.h>
#include <Chunks/ChunkStorage.h>

std::string VoxelWorld::GetName() { return "VoxelWorld"; }

std::unique_ptr<VoxelWorld> VoxelWorld::RegisterVoxelWorld() 
{
  return std::make_unique<VoxelWorld>();
}

VoxelWorld::VoxelWorld()
  : System(systemType)
{
  chunkManager_ = std::make_unique<ChunkManager>();
}

VoxelWorld::~VoxelWorld()
{
}

void VoxelWorld::Init()
{
  GetSpace()->RegisterListener(this, &VoxelWorld::UpdateEventsListen);
  Engine::GetEngine()->RegisterListener(this, &VoxelWorld::DrawEventsListen);

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

void VoxelWorld::End()
{
  if (parent != nullptr)
  {
    GetSpace()->UnregisterListener(this, &VoxelWorld::UpdateEventsListen);
    GetSpace()->UnregisterListener(this, &VoxelWorld::DrawEventsListen);
  }
}

std::unique_ptr<System> VoxelWorld::Clone() const
{
  auto result = new VoxelWorld();
  //assert(false); // do not copy this shit
  return std::unique_ptr<System>(result);
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
  glfwSwapBuffers(GraphicsManager::GetGraphicsManager()->GetWindow());
}