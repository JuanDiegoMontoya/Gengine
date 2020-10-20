#include <CoreEngine/GraphicsIncludes.h> // this needs to be first or everything breaks
#include <CoreEngine/Application.h>
#include <CoreEngine/Scene.h>
#include <CoreEngine/Entity.h>
#include <CoreEngine/Components.h>
#include <CoreEngine/Renderer.h>
#include <CoreEngine/Mesh.h>
#include <CoreEngine/Material.h>
#include <CoreEngine/Input.h>
#include <Voxels/VoxelManager.h>
#include <Voxels/ChunkSerialize.h>
#include <Voxels/ChunkRenderer.h>
#include "WorldGen.h"

// eh
#include <CoreEngine/Renderer.h>
#include <CoreEngine/Camera.h>

#include <iostream>
#include <Game/PlayerController.h>

// main.cpp: this is where the user's code belongs
static MaterialHandle userMaterial{};
static std::unique_ptr<VoxelManager> voxelManager{};

void OnStart(Scene* scene)
{
  voxelManager = std::make_unique<VoxelManager>();
  WorldGen2 wg(*voxelManager);
  wg.Init();
  wg.GenerateWorld();
  wg.InitializeSunlight();
  wg.InitMeshes();
  wg.InitBuffers();
  //auto compressed = CompressChunk(voxelManager->GetChunk(glm::ivec3(0))->GetStorage());

  MaterialInfo info;
  info.shaderID = "ShaderMcShaderFuckFace";
  info.tex2Dpaths.push_back("this is an invalid texture! (it should use a fallback)");
  userMaterial = MaterialManager::CreateMaterial(info);

  std::cout << "User function, initial scene name: " << scene->GetName() << '\n';

  {
    Entity thing = scene->CreateEntity("bun");
    thing.AddComponent<Components::Transform>();
    
    bool l, o;
    Components::Mesh mesh;
    mesh.meshHandle = MeshManager::CreateMesh("./Resources/Models/teapot.obj", l, o)[0];
    thing.AddComponent<Components::Mesh>(mesh);

    Components::Material material = userMaterial;
    thing.AddComponent<Components::Material>(material);
    //thing.GetComponent<Components::Material>().texHandle = MeshManager::GetFuckingTexture("");
  }

  {
    Entity player = scene->CreateEntity("player");
    player.AddComponent<Components::Transform>();
    player.AddComponent<Components::Camera>(Camera::ActiveCamera);
    player.AddComponent<Components::NativeScriptComponent>().Bind<PlayerController>();
  }

  // make an entity for each object in the maya mesh
  //bool l, o;
  //auto mesh = MeshManager::CreateMesh("./Resources/Models/maya.fbx", l, o);
  //for (auto handle : mesh)
  //{
  //  Entity newEnt = scene->CreateEntity("maya");
  //  newEnt.AddComponent<Components::Transform>();
  //  Components::Model model{ .model = glm::scale(glm::mat4(1), {.01, .01, .01}) };
  //  newEnt.AddComponent<Components::Model>(model);
  //  Components::Mesh mesh{ .meshHandle = handle };
  //  newEnt.AddComponent<Components::Mesh>(mesh);
  //  newEnt.AddComponent<Components::Material>(userMaterial);
  //}
}

void OnUpdate(float dt)
{
  if (Input::IsKeyDown(GLFW_KEY_LEFT_SHIFT))
  {
    if (Input::IsKeyDown(GLFW_KEY_1))
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    if (Input::IsKeyDown(GLFW_KEY_2))
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    if (Input::IsKeyDown(GLFW_KEY_3))
      glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
  }

  {
    auto& sett = voxelManager->chunkRenderer_->settings;
    if (Input::IsKeyPressed(GLFW_KEY_5))
    {
      sett.freezeCulling = !sett.freezeCulling;
      std::cout << "Freeze culling " << std::boolalpha << sett.freezeCulling << '\n';
    }
    if (Input::IsKeyPressed(GLFW_KEY_6))
    {
      // TODO: shading/wireframe on this drawing mode
      sett.debug_drawOcclusionCulling = !sett.debug_drawOcclusionCulling;
      std::cout << "DbgDrawOccCulling " << std::boolalpha << sett.debug_drawOcclusionCulling << '\n';
    }
  }
  voxelManager->Update();
}

void OnDraw(float dt)
{
  voxelManager->Draw();
  Renderer::DrawAxisIndicator();
}

int main()
{
  Application::SetStartCallback(OnStart);
  Application::SetUpdateCallback(OnUpdate);
  Application::SetDrawCallback(OnDraw);

  Application::Start();
  Application::Shutdown();
  return 0;
}