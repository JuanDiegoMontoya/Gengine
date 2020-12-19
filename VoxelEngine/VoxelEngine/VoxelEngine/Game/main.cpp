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
#include <Game/FlyingPlayerController.h>
#include <Game/PhysicsPlayerController.h>
#include <Game/PlayerActions.h>
#include <Game/TestObj.h>
#include <Game/TestObj2.h>
#include <Game/GameManager.h>
#include <Game/PhysicsTest2.h>
#include <Game/KinematicPlayerController.h>



// main.cpp: this is where the user's code belongs
static MaterialHandle userMaterial{};
static MaterialHandle batchMaterial{};
static std::unique_ptr<VoxelManager> voxelManager{};

void OnStart(Scene* scene)
{
  voxelManager = std::make_unique<VoxelManager>();
  WorldGen wg(*voxelManager);
  wg.Init();
  wg.GenerateWorld();
  wg.InitializeSunlight();
  wg.InitMeshes();
  wg.InitBuffers();
  //auto compressed = CompressChunk(voxelManager->GetChunk(glm::ivec3(0))->GetStorage());
  
  {
    MaterialInfo info;
    info.shaderID = "ShaderMcShaderFace";
    info.tex2Dpaths.push_back("==intentionally invalid texture==");
    userMaterial = MaterialManager::CreateMaterial(info);
  }
  {
    MaterialInfo info;
    info.shaderID = "batched";
    info.tex2Dpaths.push_back("==intentionally invalid texture==");
    batchMaterial = MaterialManager::CreateMaterial(info);
  }

  // add game manager entity
  {
    Entity gameManager = scene->CreateEntity("Game Manager");
    gameManager.AddComponent<Components::NativeScriptComponent>().Bind<GameManager>();
  }

  std::cout << "User function, initial scene name: " << scene->GetName() << '\n';


  {
    Entity player = scene->CreateEntity("player");
    player.AddComponent<Components::Transform>().SetRotation(glm::rotate(glm::mat4(1), glm::pi<float>() / 2.f, { 0, 0, 1 }));
    player.GetComponent<Components::Transform>().SetTranslation({ -5, 2, -5 });
    //player.AddComponent<Components::NativeScriptComponent>().Bind<FlyingPlayerController>();
    //player.AddComponent<Components::NativeScriptComponent>().Bind<PhysicsPlayerController>();
    player.AddComponent<Components::NativeScriptComponent>().Bind<KinematicPlayerController>();
    player.AddComponent<Components::Camera>(Camera::ActiveCamera);
    Physics::CapsuleCollider collider(0.3, 0.5);
    //Physics::BoxCollider collider({ 1, 1, 1 });
    player.AddComponent<Components::CharacterController>(player, Physics::MaterialType::PLAYER, collider);

    // extra stuff attached to the player
    Entity playerSub = scene->CreateEntity("PlayerSub");
    playerSub.AddComponent<Components::NativeScriptComponent>().Bind<PlayerActions>(voxelManager.get());
    player.AddChild(playerSub);

  }

  {
    bool l, o;
    std::vector<BatchedMeshHandle> meshes;
    meshes.push_back(MeshManager::CreateMeshBatched("./Resources/Models/big_cube.obj", l, o)[0]);
    meshes.push_back(MeshManager::CreateMeshBatched("./Resources/Models/bunny.obj", l, o)[0]);
    meshes.push_back(MeshManager::CreateMeshBatched("./Resources/Models/goodSphere.obj", l, o)[0]);
    meshes.push_back(MeshManager::CreateMeshBatched("./Resources/Models/teapot.obj", l, o)[0]);

    auto notbatch = MeshManager::CreateMesh("./Resources/Models/sphere.obj", l, o)[0];
    if (0) // creating a really tall parenting chain of objects
    {
      Entity parent = scene->CreateEntity("parent");
      parent.AddComponent<Components::Transform>().SetTranslation({ -15, -10, 10 });
      parent.GetComponent<Components::Transform>().SetScale({ 1, 1, 1 });
      parent.AddComponent<Components::NativeScriptComponent>().Bind<TestObj>();
      //parent.AddComponent<Components::Mesh>().meshHandle = bunny;
      //parent.AddComponent<Components::Material>(userMaterial);
      parent.AddComponent<Components::BatchedMesh>().handle = meshes[0];
      parent.AddComponent<Components::Material>(batchMaterial);

      for (int i = 0; i < 5000; i++)
      {
        Entity child = scene->CreateEntity("child");
        child.AddComponent<Components::Transform>();
        child.SetParent(parent);
        child.AddComponent<Components::NativeScriptComponent>().Bind<TestObj>();
        child.AddComponent<Components::LocalTransform>().transform.SetTranslation({ 1, 1, 1 });
        child.GetComponent<Components::LocalTransform>().transform.SetScale({ .95, .95, .95 });
        //child.AddComponent<Components::Mesh>().meshHandle = bunny;
        //child.AddComponent<Components::Material>(userMaterial);
        child.AddComponent<Components::BatchedMesh>().handle = meshes[0];
        child.AddComponent<Components::Material>(batchMaterial);
        parent = child;
      }
    }
    if (0) // instancing many objects
    {
      srand(0);
      Entity parent{};
      for (int x = 0; x < 100; x++)
      {
        for (int y = 0; y < 100; y++)
        {
          Entity entity = scene->CreateEntity("parent");
          if (!parent)
          {
            entity.AddComponent<Components::NativeScriptComponent>().Bind<TestObj>();
            parent = entity;
          }
          else
          {
            entity.SetParent(parent);
            entity.AddComponent<Components::LocalTransform>().transform.SetTranslation({ x * 3, 0, y * 3 });
            entity.AddComponent<Components::NativeScriptComponent>().Bind<TestObj2>();
          }
          entity.AddComponent<Components::Transform>().SetTranslation({ x * 3, 0, y * 3 });
          entity.GetComponent<Components::Transform>().SetScale({ 1, 1, 1 });
          entity.AddComponent<Components::BatchedMesh>().handle = meshes[rand() % meshes.size()];
          entity.AddComponent<Components::Material>(batchMaterial);
          //Components::Physics phys(entity, Physics::MaterialType::Terrain, Physics::BoxCollider(glm::vec3(1)));
          //entity.AddComponent<Components::Physics>(std::move(phys));
          //entity.AddComponent<Components::Mesh>().meshHandle = notbatch;
          //entity.AddComponent<Components::Material>(userMaterial);
        }
      }
    }
    if (1) // boxes physics test
    {
      for (int i = 0; i < 5; i++)
      {
        Entity entity = scene->CreateEntity("physics entity" + std::to_string(i));
        entity.AddComponent<Components::Transform>().SetTranslation({ -15, 50 + i, 10 + (float(i)/50.f) });
        //entity.AddComponent<Components::Transform>().SetTranslation({ -15, 50, 10 });
        glm::vec3 scale{ 1, .4f, glm::clamp(1 + i * (.02f), .1f, 10.f) };
        entity.GetComponent<Components::Transform>().SetScale(scale);
        entity.AddComponent<Components::BatchedMesh>().handle = meshes[0];
        entity.AddComponent<Components::Material>(batchMaterial);
        auto collider = Physics::BoxCollider(scale * .5f);
        Components::DynamicPhysics phys(entity, Physics::MaterialType::TERRAIN, collider);
        entity.AddComponent<Components::DynamicPhysics>(std::move(phys));
      }
    }
    if (0) // spheres physics test
    {
      for (int i = 0; i < 500; i++)
      {
        Entity entity = scene->CreateEntity("physics entity" + std::to_string(i));
        entity.AddComponent<Components::Transform>().SetTranslation({ -10, 50 + i, 10 + (float(i) / 50.f) });
        //entity.AddComponent<Components::Transform>().SetTranslation({ -15, 50, 10 });
        glm::vec3 scale{ 1, 1, 1 };
        entity.GetComponent<Components::Transform>().SetScale(scale * .5f);
        entity.AddComponent<Components::BatchedMesh>().handle = meshes[2];
        entity.AddComponent<Components::Material>(batchMaterial);
        auto collider = Physics::CapsuleCollider(.5, .01);
        Components::DynamicPhysics phys(entity, Physics::MaterialType::TERRAIN, collider);
        entity.AddComponent<Components::DynamicPhysics>(std::move(phys));
      }
    }
    if (0) // interactive physics test
    {
      Entity entity = scene->CreateEntity("controlled physics entity");
      entity.AddComponent<Components::Transform>().SetTranslation({ -15, 5, 10 });
      entity.GetComponent<Components::Transform>().SetScale({ 1, 1, 1 });
      entity.AddComponent<Components::BatchedMesh>().handle = meshes[0];
      entity.AddComponent<Components::Material>(batchMaterial);
      Components::DynamicPhysics phys(entity, Physics::MaterialType::TERRAIN, Physics::BoxCollider(glm::vec3(.5)));
      entity.AddComponent<Components::DynamicPhysics>(std::move(phys));
      entity.AddComponent<Components::NativeScriptComponent>().Bind<PhysicsTest2>();
    }
    if (0) // stack of static entities test
    {
      for (int i = 0; i < 10; i++)
      {
        for (int j = i; j < 10; j++)
        {
          Entity entity = scene->CreateEntity("physics entity" + std::to_string(i));
          entity.AddComponent<Components::Transform>().SetTranslation({ -15, 0 + i, (float(i)/2.f + j-i) });
          glm::vec3 scale{ 1, 1, 1 };
          entity.GetComponent<Components::Transform>().SetScale(scale);
          entity.AddComponent<Components::BatchedMesh>().handle = meshes[0];
          entity.AddComponent<Components::Material>(batchMaterial);
          auto collider = Physics::BoxCollider({ .5, .5, .5 });
          Components::StaticPhysics phys(entity, Physics::MaterialType::TERRAIN, collider);
          entity.AddComponent<Components::StaticPhysics>(std::move(phys));
        }
      }
    }
    if (0) // static mesh physics test
    {
      Physics::MeshCollider collider;
      collider.vertices = { {-.5, -.5, -.5}, {-.5, -.5, .5}, {.5, -.5, .5}, {.5, -.5, -.5}, {0, .5, 0} };
      collider.indices = { 0, 1, 3, 3, 1, 2, 0, 4, 3, 3, 4, 2, 2, 4, 1, 1, 4, 0 };
      auto* actor = Physics::PhysicsManager::AddStaticActorGeneric(Physics::MaterialType::TERRAIN, collider, glm::mat4(1));
      ASSERT(actor);
    }
  }


  // make an entity for each object in the maya mesh
  //bool l, o;
  //auto mesh = MeshManager::CreateMesh("./Resources/Models/Knuckles.fbx", l, o);
  //for (auto handle : mesh)
  //{
  //  Entity newEnt = scene->CreateEntity("maya");
  //  Components::Transform model;// {.model = glm::scale(glm::mat4(1), { .01, .01, .01 }) };
  //  model.SetScale({ .01, .01, .01 });
  //  newEnt.AddComponent<Components::Transform>(model);
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

#include <Voxels/prefab.h>
int main()
{
  PrefabManager::InitPrefabs();
  Application::SetStartCallback(OnStart);
  Application::SetUpdateCallback(OnUpdate);
  Application::SetDrawCallback(OnDraw);

  Application::Start();
  Application::Shutdown();

  return 0;
}