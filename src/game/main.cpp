#include "gPCH.h"
// #include <glad/glad.h>
#include <engine/Application.h>
#include <engine/Scene.h>
#include <engine/ecs/Entity.h>
#include <engine/gfx/Renderer.h>
#include <engine/gfx/resource/MeshManager.h>
#include <engine/gfx/resource/MaterialManager.h>
#include <engine/Input.h>
#include <voxel/VoxelManager.h>
#include <voxel/ChunkSerialize.h>
#include <voxel/ChunkRenderer.h>
#include "WorldGen.h"
#include <engine/gfx/api/Texture.h>
#include <engine/gfx/resource/TextureManager.h>
#include <engine/gfx/TextureLoader.h>

#include <utility/MathExtensions.h>
#include <utility/Timer.h>
#include <glm/gtc/type_ptr.hpp>

// eh
#include <engine/gfx/Renderer.h>
#include <voxel/prefab.h>
#include <engine/ecs/system/ParticleSystem.h>

#include <iostream>
#include <game/FlyingPlayerController.h>
#include <game/PhysicsPlayerController.h>
#include <game/PlayerActions.h>
#include <game/TestObj.h>
#include <game/TestObj2.h>
#include <game/GameManager.h>
#include <game/PhysicsTest2.h>
#include <game/KinematicPlayerController.h>

#include <engine/ecs/component/Transform.h>
#include <engine/ecs/component/Core.h>
#include <engine/ecs/component/Rendering.h>
#include <engine/ecs/component/Physics.h>
#include <engine/ecs/component/Scripting.h>
#include <engine/ecs/component/ParticleEmitter.h>

static std::unique_ptr<Voxels::VoxelManager> voxelManager{};

void OnStart(Scene* scene)
{
  // TODO: eventually remove this
  Voxels::PrefabManager::InitPrefabs();
  voxelManager = std::make_unique<Voxels::VoxelManager>(scene);
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

  auto mySetter = [](hashed_string id, GFX::Shader& shader) { shader.SetFloat(id, (float)ProgramTimer::TimeSeconds()); };

  GFX::TextureManager::AddTexture("error", *GFX::LoadTexture2D("error/diffuse.png"));
  auto view = GFX::TextureView::Create(*GFX::TextureManager::GetTexture("error"), "batched material view");
  auto sampler = GFX::TextureSampler::Create(GFX::SamplerState{}, "batched material sampler");
  GFX::PerMaterialUniformData uniformData;
  uniformData.id = "u_time";
  uniformData.Setter = mySetter;
  GFX::MaterialCreateInfo info
  {
    .shaderID = "batched",
    .viewSamplers = {{std::move(*view), std::move(*sampler)}},
    .materialUniforms = { uniformData },
  };
  GFX::MaterialID batchMaterial = GFX::MaterialManager::AddMaterial("batchMaterial", info);

  // add game manager entity
  {
    Entity gameManager = scene->CreateEntity("Game Manager");
    gameManager.AddComponent<Component::NativeScriptComponent>().Bind<GameManager>();
  }

  GFX::TextureManager::AddTexture("smoke", *GFX::LoadTexture2D("smoke.png"));
  GFX::TextureManager::AddTexture("stone", *GFX::LoadTexture2D("Voxel/stone/diffuse.png"));

  using RMB = GFX::RenderMaskBit;
  GFX::RenderMask mask = 
    RMB::RenderSky |
    RMB::RenderFog |
    RMB::RenderEmitters |
    RMB::RenderObjects |
    RMB::RenderScreenElements |
    RMB::RenderVoxels;

  auto* mainRenderView = GFX::Renderer::GetMainRenderView();
  //mainRenderView->camera->proj = MakeInfReversedZProjRH(glm::radians(70.0f), GFX::Renderer::GetWindowAspectRatio(), 0.1f);
  GFX::PerspectiveProjectionInfo projInfo
  {
    .fovyRadians = glm::radians(70.0f),
    .aspectRatio = GFX::Renderer::GetWindowAspectRatio(),
    .nearPlane = 0.1f
  };
  mainRenderView->camera->projInfo.info = projInfo;
  mainRenderView->camera->viewInfo.position = { 0, 2, 0 };
  mainRenderView->mask = mask;
  mainRenderView->renderInfo.offset = { 0, 0 };
  mainRenderView->renderInfo.size = GFX::Renderer::GetWindowDimensions();

  GFX::RenderMask probeMask =
    RMB::RenderSky |
    RMB::RenderEarlyFog |
    RMB::RenderVoxels |
    RMB::RenderVoxelsNear |
    RMB::RenderObjects;
  GFX::Renderer::SetProbeRenderMask(probeMask);

  //auto view2 = *GFX::TextureView::Create(*probeFaces[0].colorMemory);
  //auto sampler2 = *GFX::TextureSampler::Create({});
  //GFX::MaterialCreateInfo info2
  //{
  //  .shaderID = "batched",
  //  .viewSamplers = {{std::move(view2), std::move(sampler2)}},
  //  .materialUniforms = { uniformData }
  //};
  //GFX::MaterialID testMaterial = GFX::MaterialManager::Get()->AddMaterial("testMaterial", info2);

  {
    Entity player = scene->CreateEntity("player");
    player.AddComponent<Component::Transform>().SetRotation(glm::rotate(glm::mat4(1), glm::pi<float>() / 2.f, { 0, 0, 1 }));
    player.GetComponent<Component::Transform>().SetTranslation({ -5, 2, -5 });
    //player.AddComponent<Components::NativeScriptComponent>().Bind<FlyingPlayerController>();
    //player.AddComponent<Components::NativeScriptComponent>().Bind<PhysicsPlayerController>();
    player.AddComponent<Component::NativeScriptComponent>().Bind<KinematicPlayerController>();
    //player.AddComponent<Components::Camera>(Camera::ActiveCamera);
    //cam.skybox = std::make_unique<GFX::TextureCube>(std::span<const std::string, 6>(faces.data(), faces.size()));

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

    std::vector<GFX::MeshID> meshes;
    meshes.push_back(GFX::MeshManager::CreateMeshBatched(std::string(ModelDir) + "big_cube.obj", "big_cube"));
    meshes.push_back(GFX::MeshManager::CreateMeshBatched(std::string(ModelDir) + "bunny.obj", "bunny"));
    meshes.push_back(GFX::MeshManager::CreateMeshBatched(std::string(ModelDir) + "goodSphere.obj", "sphere"));
    meshes.push_back(GFX::MeshManager::CreateMeshBatched(std::string(ModelDir) + "teapot.obj", "teapot"));

    Entity testCam = scene->CreateEntity("testCam");
    testCam.AddComponent<Component::Transform>().SetTranslation({ 0, 0, 0 });
    testCam.AddComponent<Component::Model>();
    testCam.AddComponent<Component::BatchedMesh>().handle = GFX::MeshManager::GetMeshBatched("big_cube");
    testCam.AddComponent<Component::Material>(batchMaterial);

    if (0) // creating a really tall parenting chain of objects
    {
      Entity parent = scene->CreateEntity("parentBigly");
      parent.AddComponent<Component::Transform>().SetTranslation({ -15, -10, 10 });
      parent.AddComponent<Component::Model>();
      parent.GetComponent<Component::Transform>().SetScale({ 1, 1, 1 });
      parent.AddComponent<Component::NativeScriptComponent>().Bind<TestObj>();
      //parent.AddComponent<Components::Mesh>().meshHandle = bunny;
      //parent.AddComponent<Components::Material>(userMaterial);
      parent.AddComponent<Component::BatchedMesh>().handle = GFX::MeshManager::GetMeshBatched("big_cube");
      parent.AddComponent<Component::Material>(GFX::MaterialManager::GetMaterial("batchMaterial"));

      for (int i = 0; i < 1000; i++)
      {
        Entity child = scene->CreateEntity("child");
        child.AddComponent<Component::Transform>();
        child.AddComponent<Component::Model>();
        child.SetParent(parent);
        child.AddComponent<Component::NativeScriptComponent>().Bind<TestObj>();
        child.AddComponent<Component::LocalTransform>().transform.SetTranslation({ 1, 1, 1 });
        child.GetComponent<Component::LocalTransform>().transform.SetScale({ .95, .95, .95 });
        //child.AddComponent<Components::Mesh>().meshHandle = bunny;
        //child.AddComponent<Components::Material>(userMaterial);
        child.AddComponent<Component::BatchedMesh>().handle = GFX::MeshManager::GetMeshBatched("big_cube");
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
          entity.AddComponent<Component::Model>();
          //Components::Physics phys(entity, Physics::MaterialType::Terrain, Physics::BoxCollider(glm::vec3(1)));
          //entity.AddComponent<Components::Physics>(std::move(phys));
          //entity.AddComponent<Components::Mesh>().meshHandle = notbatch;
          //entity.AddComponent<Components::Material>(userMaterial);
        }
      }
    }
    if (0) // boxes physics test
    {
      for (int i = 0; i < 500; i++)
      {
        Entity entity = scene->CreateEntity("physics entity" + std::to_string(i));
        entity.AddComponent<Component::Transform>().SetTranslation({ 35, 50 + i, 30 + (float(i) / 50.f) });
        //entity.AddComponent<Components::Transform>().SetTranslation({ -15, 50, 10 });
        glm::vec3 scale{ 1, .4f, glm::clamp(1 + i * (.02f), .1f, 10.f) };
        entity.GetComponent<Component::Transform>().SetScale(scale);
        entity.AddComponent<Component::BatchedMesh>().handle = GFX::MeshManager::GetMeshBatched("big_cube");
        entity.AddComponent<Component::Material>().handle = GFX::MaterialManager::GetMaterial("batchMaterial");
        entity.AddComponent<Component::Model>();
        entity.AddComponent<Component::InterpolatedPhysics>();
        auto collider = Physics::BoxCollider(scale * .5f);
        Component::DynamicPhysics phys(entity, Physics::MaterialType::TERRAIN, collider);
        entity.AddComponent<Component::DynamicPhysics>(std::move(phys));
      }
    }
    if (0) // spheres physics test
    {
      for (int i = 0; i < 50; i++)
      {
        Entity entity = scene->CreateEntity("physics entity" + std::to_string(i));
        entity.AddComponent<Component::Transform>().SetTranslation({ 30, 50 + i, 30 + (float(i) / 50.f) });
        //entity.AddComponent<Components::Transform>().SetTranslation({ -15, 50, 10 });
        glm::vec3 scale{ 1, 1, 1 };
        entity.GetComponent<Component::Transform>().SetScale(scale * .5f);
        entity.AddComponent<Component::BatchedMesh>().handle = meshes[2];
        entity.AddComponent<Component::Material>().handle = batchMaterial;
        entity.AddComponent<Component::Model>();
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
      entity.AddComponent<Component::Model>();
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
          entity.AddComponent<Component::Model>();
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
      entity.AddComponent<Component::BatchedMesh>().handle = GFX::MeshManager::GetMeshBatched("teapot");
      //mesh.renderFlag = (uint64_t)RenderFlags::NoRender;
      entity.AddComponent<Component::Material>().handle = batchMaterial;
      entity.AddComponent<Component::Model>();
      //entity.AddComponent<Components::NativeScriptComponent>().Bind<TestObj>();
      Component::ParticleEmitter emitter;
      emitter.handle = ParticleManager::Get().MakeParticleEmitter(150, "stone");
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
      entity.AddComponent<Component::ParticleEmitter>(emitter);
    }
    if (1) // rain particle test
    {
      Entity entity = scene->CreateEntity("rainmaker");
      entity.AddComponent<Component::Transform>();
      entity.AddComponent<Component::LocalTransform>();
      scene->GetEntity("player").AddChild(entity);
      Component::ParticleEmitter emitter;

      // rain
      emitter.handle = ParticleManager::Get().MakeParticleEmitter(1'000'000, "smoke");
      emitter.data.minLife = 0.5f;
      emitter.data.maxLife = 1.5f;
      emitter.data.interval = 1.000001f;
      emitter.data.minParticleAccel = { 0, 0, 0 };
      emitter.data.maxParticleAccel = { 0, 0, 0 };
      emitter.data.minParticleVelocity = { -1, -10, -1 };
      emitter.data.maxParticleVelocity = { 1, -9, 1 };
      emitter.data.minParticleOffset = { -20, 0, -20 };
      emitter.data.maxParticleOffset = { 20, 20, 20 };
      emitter.data.minParticleScale = { .005, .02 };
      emitter.data.maxParticleScale = { .01, .03 };
      emitter.data.minParticleColor = { .4, .4, 1, .2 };
      emitter.data.maxParticleColor = { .7, .7, 1, .4 };

      // snow
      //emitter.handle = ParticleManager::Get().MakeParticleEmitter(100'000, "smoke");
      //emitter.data.minLife = 1;
      //emitter.data.maxLife = 2;
      //emitter.data.interval = .00002f;
      //emitter.data.minParticleAccel = { 0, 0, 0 };
      //emitter.data.maxParticleAccel = { 0, 0, 0 };
      //emitter.data.minParticleVelocity = { -.5, -1, -.5 };
      //emitter.data.maxParticleVelocity = { .1, -.5, .1 };
      //emitter.data.minParticleOffset = { -20, 0, -20 };
      //emitter.data.maxParticleOffset = { 20, 20, 20 };
      //emitter.data.minParticleScale = { .02, .02 };
      //emitter.data.maxParticleScale = { .03, .03 };
      //emitter.data.minParticleColor = { .6, .6, .6, .6 };
      //emitter.data.maxParticleColor = { .7, .7, .7, .8 };

      //emitter.data.minLife = 0.5f;
      //emitter.data.maxLife = 1.5f;
      //emitter.data.interval = .01f;
      //emitter.data.minParticleAccel = { 0, 0, 0 };
      //emitter.data.maxParticleAccel = { 0, 0, 0 };
      //emitter.data.minParticleVelocity = { 0, 0, 0 };
      //emitter.data.maxParticleVelocity = { 0, 0, 0 };
      //emitter.data.minParticleOffset = { 0, 0, 0 };
      //emitter.data.maxParticleOffset = { 2, 2, 2 };
      //emitter.data.minParticleScale = { .1, .1 };
      //emitter.data.maxParticleScale = { .1, .1 };
      //emitter.data.minParticleColor = { .4, .4, 1, 1 };
      //emitter.data.maxParticleColor = { .7, .7, 1, 1 };

      entity.AddComponent<Component::ParticleEmitter>(emitter);
    }
  }
}

#include <imgui/imgui.h>
void OnUpdate([[maybe_unused]] Timestep timestep)
{
  voxelManager->Update();
}

void OnDraw(Scene* scene, [[maybe_unused]] Timestep timestep)
{
  static bool freezeCams = false;
  //auto& vi = scene->GetRenderView("test").camera->viewInfo;
  ImGui::Checkbox("Freeze Cams", &freezeCams);
  if (!freezeCams)
  {
    glm::vec3 pos = scene->GetEntity("player").GetComponent<Component::Transform>().GetTranslation();
    pos.y += 0.65f;
    GFX::Renderer::SetProbePosition(pos);
  }
  //ImGui::SliderFloat("Pitch", &vi.pitch, -glm::pi<float>() / 2, glm::pi<float>() / 2);
  //ImGui::SliderFloat("Yaw", &vi.yaw, 0, 6.28);
  //ImGui::SliderFloat3("Pos", glm::value_ptr(vi.position), 0, 10);
  auto renderViews = scene->GetRenderViews();
  voxelManager->Draw();
}

void OnInterfaceDraw([[maybe_unused]] Scene* scene, [[maybe_unused]] Timestep timestep)
{
  auto renderViews = scene->GetRenderViews();
  GFX::Renderer::DrawAxisIndicator(renderViews);
  voxelManager->DrawDebug();
}

int main()
{
  engine::Core::InitLogging();

  Application::SetStartCallback(OnStart);
  Application::SetUpdateCallback(OnUpdate);
  Application::SetDrawOpaqueCallback(OnDraw);
  Application::SetDrawInterfacePostPostProcessingCallback(OnInterfaceDraw);

  Application::Start();
  voxelManager.reset();
  Application::Shutdown();

  return 0;
}