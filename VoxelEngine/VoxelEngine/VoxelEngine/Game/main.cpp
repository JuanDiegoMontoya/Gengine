#include <CoreEngine/GraphicsIncludes.h> // this needs to be first or everything breaks
#include <CoreEngine/Application.h>
#include <CoreEngine/Scene.h>
#include <CoreEngine/Entity.h>
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
#include <Voxels/prefab.h>
#include <CoreEngine/TextureCube.h>
#include <CoreEngine/ParticleSystem.h>

#include <iostream>
#include <Game/FlyingPlayerController.h>
#include <Game/PhysicsPlayerController.h>
#include <Game/PlayerActions.h>
#include <Game/TestObj.h>
#include <Game/TestObj2.h>
#include <Game/GameManager.h>
#include <Game/PhysicsTest2.h>
#include <Game/KinematicPlayerController.h>

#include <CoreEngine/Components/Transform.h>
#include <CoreEngine/Components/Core.h>
#include <CoreEngine/Components/Rendering.h>
#include <CoreEngine/Components/Physics.h>
#include <CoreEngine/Components/Scripting.h>
#include <CoreEngine/Components/Camera.h>
#include <CoreEngine/Components/ParticleEmitter.h>

static std::unique_ptr<Voxels::VoxelManager> voxelManager{};

void OnStart(Scene* scene)
{
  // TODO: eventually remove this
  Voxels::PrefabManager::InitPrefabs();
  voxelManager = std::make_unique<Voxels::VoxelManager>(*scene);
  WorldGen wg(*voxelManager);
  wg.Init();
  wg.GenerateWorld();
//#if !DEBUG
  wg.InitializeSunlight();
//#endif
  wg.InitMeshes();
  wg.InitBuffers();
  //auto compressed = CompressChunk(voxelManager->GetChunk(glm::ivec3(0))->GetStorage());

  InputAxisType attackButtons[] = { {.scale = 1.0f, .type = InputMouseButton{.button = GLFW_MOUSE_BUTTON_1 }} };
  InputActionType buildButtons[] = { InputMouseButton{.button = GLFW_MOUSE_BUTTON_2 } };
  InputAxisType crouchButtons[] = { {.scale = 1.0f, .type = InputKey{.key = GLFW_KEY_LEFT_CONTROL}} };
  InputAxisType sprintButtons[] = { {.scale = 1.0f, .type = InputKey{.key = GLFW_KEY_LEFT_SHIFT}} };
  InputAxisType strafeAxes[] = {
    {.scale = -1.0f, .type = InputKey{.key = GLFW_KEY_A }},
    {.scale = 1.0f, .type = InputKey{.key = GLFW_KEY_D }} };
  InputAxisType forwardAxes[] = {
    {.scale = -1.0f, .type = InputKey{.key = GLFW_KEY_S }},
    {.scale = 1.0f, .type = InputKey{.key = GLFW_KEY_W }} };
  Input::AddInputAxis("Attack", attackButtons);
  Input::AddInputAction("Build", buildButtons);
  Input::AddInputAxis("Strafe", strafeAxes);
  Input::AddInputAxis("Forward", forwardAxes);
  Input::AddInputAxis("Sprint", sprintButtons);
  Input::AddInputAxis("Crouch", crouchButtons);

  MaterialInfo info{};
  info.shaderID = "batched";
  info.tex2Dpaths.push_back("==intentionally invalid texture==");
  auto batchMaterial = MaterialManager::CreateMaterial(info, "batchMaterial");

  // add game manager entity
  {
    Entity gameManager = scene->CreateEntity("Game Manager");
    gameManager.AddComponent<Component::NativeScriptComponent>().Bind<GameManager>();
  }

  std::cout << "User function, initial scene name: " << scene->GetName() << '\n';


  //const std::vector<std::string> faces =
  //{
  //  "hw_glacier/glacier_rt.tga",
  //  "hw_glacier/glacier_lf.tga",
  //  "hw_glacier/glacier_up.tga",
  //  "hw_glacier/glacier_dn.tga",
  //  "hw_glacier/glacier_bk.tga",
  //  "hw_glacier/glacier_ft.tga",
  //};
  //const std::vector<std::string> faces =
  //{
  //  "night_sky_hdr/px.hdr",
  //  "night_sky_hdr/nx.hdr",
  //  "night_sky_hdr/py.hdr",
  //  "night_sky_hdr/ny.hdr",
  //  "night_sky_hdr/pz.hdr",
  //  "night_sky_hdr/nz.hdr",
  //};
  const std::vector<std::string> faces =
  {
    "autumn_sky_hdr/px.hdr",
    "autumn_sky_hdr/nx.hdr",
    "autumn_sky_hdr/py.hdr",
    "autumn_sky_hdr/ny.hdr",
    "autumn_sky_hdr/pz.hdr",
    "autumn_sky_hdr/nz.hdr",
  };

  //const std::vector<std::string> faces =
  //{
  //  "miramar/rt.tga",
  //  "miramar/lf.tga",
  //  "miramar/up.tga",
  //  "miramar/dn.tga",
  //  "miramar/bk.tga",
  //  "miramar/ft.tga",
  //};
  {
    Entity player = scene->CreateEntity("player");
    player.AddComponent<Component::Transform>().SetRotation(glm::rotate(glm::mat4(1), glm::pi<float>() / 2.f, { 0, 0, 1 }));
    player.GetComponent<Component::Transform>().SetTranslation({ -5, 2, -5 });
    //player.AddComponent<Components::NativeScriptComponent>().Bind<FlyingPlayerController>();
    //player.AddComponent<Components::NativeScriptComponent>().Bind<PhysicsPlayerController>();
    player.AddComponent<Component::NativeScriptComponent>().Bind<KinematicPlayerController>();
    //player.AddComponent<Components::Camera>(Camera::ActiveCamera);
    auto& cam = player.AddComponent<Component::Camera>(player);
    cam.skybox = std::make_unique<GFX::TextureCube>(std::span<const std::string, 6>(faces.data(), faces.size()));
    cam.SetPos({ 0, .65, 0 });
    Physics::CapsuleCollider collider(0.3, 0.5);
    //Physics::BoxCollider collider({ 1, 1, 1 });
    player.AddComponent<Component::CharacterController>(player, Physics::MaterialType::PLAYER, collider);

    // extra stuff attached to the player
    Entity playerSub = scene->CreateEntity("PlayerSub");
    playerSub.AddComponent<Component::NativeScriptComponent>().Bind<PlayerActions>(voxelManager.get());
    player.AddChild(playerSub);

  }

  {
    //std::vector<MeshHandle> meshes;
    //meshes.push_back(MeshManager::CreateMeshBatched("./Resources/Models/big_cube.obj"));
    //meshes.push_back(MeshManager::CreateMeshBatched("./Resources/Models/bunny.obj"));
    //meshes.push_back(MeshManager::CreateMeshBatched("./Resources/Models/goodSphere.obj"));
    //meshes.push_back(MeshManager::CreateMeshBatched("./Resources/Models/teapot.obj"));

    //auto notbatch = MeshManager::CreateMesh("./Resources/Models/sphere.obj", l, o)[0];

    std::vector<MeshID> meshes;
    meshes.push_back(MeshManager::CreateMeshBatched("./Resources/Models/big_cube.obj", "big_cube"));
    meshes.push_back(MeshManager::CreateMeshBatched("./Resources/Models/bunny.obj", "bunny"));
    meshes.push_back(MeshManager::CreateMeshBatched("./Resources/Models/goodSphere.obj", "sphere"));
    meshes.push_back(MeshManager::CreateMeshBatched("./Resources/Models/teapot.obj", "teapot"));

    if (0) // creating a really tall parenting chain of objects
    {
      Entity parent = scene->CreateEntity("parent");
      parent.AddComponent<Component::Transform>().SetTranslation({ -15, -10, 10 });
      parent.GetComponent<Component::Transform>().SetScale({ 1, 1, 1 });
      parent.AddComponent<Component::NativeScriptComponent>().Bind<TestObj>();
      //parent.AddComponent<Components::Mesh>().meshHandle = bunny;
      //parent.AddComponent<Components::Material>(userMaterial);
      parent.AddComponent<Component::BatchedMesh>().handle = MeshManager::GetMeshBatched("big_cube");
      parent.AddComponent<Component::Material>(MaterialManager::GetMaterial("batchMaterial"));

      for (int i = 0; i < 5000; i++)
      {
        Entity child = scene->CreateEntity("child");
        child.AddComponent<Component::Transform>();
        child.SetParent(parent);
        child.AddComponent<Component::NativeScriptComponent>().Bind<TestObj>();
        child.AddComponent<Component::LocalTransform>().transform.SetTranslation({ 1, 1, 1 });
        child.GetComponent<Component::LocalTransform>().transform.SetScale({ .95, .95, .95 });
        //child.AddComponent<Components::Mesh>().meshHandle = bunny;
        //child.AddComponent<Components::Material>(userMaterial);
        child.AddComponent<Component::BatchedMesh>().handle = MeshManager::GetMeshBatched("big_cube");
        child.AddComponent<Component::Material>().handle = batchMaterial;
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
            entity.AddComponent<Component::NativeScriptComponent>().Bind<TestObj>();
            parent = entity;
          }
          else
          {
            entity.SetParent(parent);
            entity.AddComponent<Component::LocalTransform>().transform.SetTranslation({ x * 3, 0, y * 3 });
            entity.AddComponent<Component::NativeScriptComponent>().Bind<TestObj2>();
          }
          entity.AddComponent<Component::Transform>().SetTranslation({ x * 3, 0, y * 3 });
          entity.GetComponent<Component::Transform>().SetScale({ 1, 1, 1 });
          entity.AddComponent<Component::BatchedMesh>().handle = meshes[rand() % meshes.size()];
          entity.AddComponent<Component::Material>().handle = batchMaterial;
          //Components::Physics phys(entity, Physics::MaterialType::Terrain, Physics::BoxCollider(glm::vec3(1)));
          //entity.AddComponent<Components::Physics>(std::move(phys));
          //entity.AddComponent<Components::Mesh>().meshHandle = notbatch;
          //entity.AddComponent<Components::Material>(userMaterial);
        }
      }
    }
    if (1) // boxes physics test
    {
      for (int i = 0; i < 50; i++)
      {
        Entity entity = scene->CreateEntity("physics entity" + std::to_string(i));
        entity.AddComponent<Component::Transform>().SetTranslation({ 35, 50 + i, 30 + (float(i) / 50.f) });
        //entity.AddComponent<Components::Transform>().SetTranslation({ -15, 50, 10 });
        glm::vec3 scale{ 1, .4f, glm::clamp(1 + i * (.02f), .1f, 10.f) };
        entity.GetComponent<Component::Transform>().SetScale(scale);
        entity.AddComponent<Component::BatchedMesh>().handle = MeshManager::GetMeshBatched("big_cube");
        entity.AddComponent<Component::Material>().handle = MaterialManager::GetMaterial("batchMaterial");
        auto collider = Physics::BoxCollider(scale * .5f);
        Component::DynamicPhysics phys(entity, Physics::MaterialType::TERRAIN, collider);
        entity.AddComponent<Component::DynamicPhysics>(std::move(phys));
      }
    }
    if (1) // spheres physics test
    {
      for (int i = 0; i < 50; i++)
      {
        Entity entity = scene->CreateEntity("physics entity" + std::to_string(i));
        entity.AddComponent<Component::Transform>().SetTranslation({ +30, 50 + i, 30 + (float(i) / 50.f) });
        //entity.AddComponent<Components::Transform>().SetTranslation({ -15, 50, 10 });
        glm::vec3 scale{ 1, 1, 1 };
        entity.GetComponent<Component::Transform>().SetScale(scale * .5f);
        entity.AddComponent<Component::BatchedMesh>().handle = meshes[2];
        entity.AddComponent<Component::Material>().handle = batchMaterial;
        auto collider = Physics::CapsuleCollider(.5, .01);
        Component::DynamicPhysics phys(entity, Physics::MaterialType::TERRAIN, collider);
        entity.AddComponent<Component::DynamicPhysics>(std::move(phys));
      }
    }
    if (0) // interactive physics test
    {
      Entity entity = scene->CreateEntity("controlled physics entity");
      entity.AddComponent<Component::Transform>().SetTranslation({ -15, 5, 10 });
      entity.GetComponent<Component::Transform>().SetScale({ 1, 1, 1 });
      entity.AddComponent<Component::BatchedMesh>().handle = meshes[0];
      entity.AddComponent<Component::Material>().handle = batchMaterial;
      Component::DynamicPhysics phys(entity, Physics::MaterialType::TERRAIN, Physics::BoxCollider(glm::vec3(.5)));
      entity.AddComponent<Component::DynamicPhysics>(std::move(phys));
      entity.AddComponent<Component::NativeScriptComponent>().Bind<PhysicsTest2>();
    }
    if (0) // stack of static entities test
    {
      for (int i = 0; i < 10; i++)
      {
        for (int j = i; j < 10; j++)
        {
          Entity entity = scene->CreateEntity("physics entity" + std::to_string(i));
          entity.AddComponent<Component::Transform>().SetTranslation({ -15, 0 + i, (float(i) / 2.f + j - i) });
          glm::vec3 scale{ 1, 1, 1 };
          entity.GetComponent<Component::Transform>().SetScale(scale);
          entity.AddComponent<Component::BatchedMesh>().handle = meshes[0];
          entity.AddComponent<Component::Material>(batchMaterial);
          auto collider = Physics::BoxCollider({ .5, .5, .5 });
          Component::StaticPhysics phys(entity, Physics::MaterialType::TERRAIN, collider);
          entity.AddComponent<Component::StaticPhysics>(std::move(phys));
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
    if (1) // particle emitter test
    {
      Entity entity = scene->CreateEntity("particle boi");
      auto& tr = entity.AddComponent<Component::Transform>();
      tr.SetTranslation({ 2, 0, -2 });
      tr.SetScale({ 1, 1, 1 });
      auto& mesh = entity.AddComponent<Component::BatchedMesh>();
      mesh.handle = MeshManager::GetMeshBatched("teapot");
      //mesh.renderFlag = (uint64_t)RenderFlags::NoRender;
      entity.AddComponent<Component::Material>().handle = batchMaterial;
      //entity.AddComponent<Components::NativeScriptComponent>().Bind<TestObj>();
      Component::ParticleEmitter emitter;
      emitter.handle = ParticleManager::MakeParticleEmitter(150, "stone.png");
#if 1
      emitter.data.minLife = 1.0f;
      emitter.data.maxLife = 2.0f;
      emitter.data.interval = .01;
      emitter.data.minParticleAccel = { -1, -1, -1 };
      emitter.data.maxParticleAccel = { 1, 1, 1 };
      emitter.data.minParticleScale = { .01, .01 };
      emitter.data.maxParticleScale = { .05, .05 };
      emitter.data.minParticleColor = { .75, .75, 0, .4 };
      emitter.data.maxParticleColor = { 1, .75, 0, .8 };
#else
      emitter.data.minLife = 1.0f;
      emitter.data.maxLife = 1.0f;
      emitter.data.interval = .5f;
      emitter.data.minParticleAccel = { 0, 0, 0 };
      emitter.data.maxParticleAccel = { 0, 0, 0 };
      emitter.data.minParticleScale = { 1, 1 };
      emitter.data.maxParticleScale = { 1, 1 };
      emitter.data.minParticleColor = { .75, .75, 0, .4 };
      emitter.data.maxParticleColor = { 1, .75, 0, .8 };
#endif
      //emitter.renderFlag = (uint64_t)RenderFlags::NoRender;
      entity.AddComponent<Component::ParticleEmitter>(std::move(emitter));
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

int main()
{
  Application::SetStartCallback(OnStart);
  Application::SetUpdateCallback(OnUpdate);
  Application::SetDrawCallback(OnDraw);

  Application::Start();
  voxelManager.reset();
  Application::Shutdown();

  return 0;
}