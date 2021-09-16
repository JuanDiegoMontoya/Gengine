#include "gPCH.h"
// #include <glad/glad.h>
#include <engine/Application.h>
#include <engine/Scene.h>
#include <engine/ecs/Entity.h>
#include <engine/gfx/Renderer.h>
#include <engine/gfx/Mesh.h>
#include <engine/gfx/Material.h>
#include <engine/Input.h>
#include <voxel/VoxelManager.h>
#include <voxel/ChunkSerialize.h>
#include <voxel/ChunkRenderer.h>
#include "WorldGen.h"
#include <engine/gfx/Texture.h>
#include <engine/gfx/TextureManager.h>
#include <engine/gfx/TextureLoader.h>

#include <utility/MathExtensions.h>
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
static GFX::Camera mainCamera;

struct ProbeFaceInfo
{
  GFX::Camera camera;
  std::optional<GFX::Framebuffer> fbo;
  std::optional<GFX::TextureView> colorView;
  std::optional<GFX::TextureView> depthView;
};

static ProbeFaceInfo probeFaces[6];

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

  GFX::TextureManager::Get()->AddTexture("error", *GFX::LoadTexture2D("error.png"));
  auto view = *GFX::TextureView::Create(*GFX::TextureManager::Get()->GetTexture("error"), "batched material view");
  auto sampler = *GFX::TextureSampler::Create(GFX::SamplerState{}, "batched material sampler");
  GFX::PerMaterialUniformData uniformData;
  uniformData.id = "u_time";
  uniformData.Setter = mySetter;
  GFX::MaterialCreateInfo info
  {
    .shaderID = "batched",
    .viewSamplers = {{std::move(view), std::move(sampler)}},
    .materialUniforms = { uniformData },
  };
  GFX::MaterialID batchMaterial = GFX::MaterialManager::Get()->AddMaterial("batchMaterial", info);

  // add game manager entity
  {
    Entity gameManager = scene->CreateEntity("Game Manager");
    gameManager.AddComponent<Component::NativeScriptComponent>().Bind<GameManager>();
  }

  GFX::TextureManager::Get()->AddTexture("smoke", *GFX::LoadTexture2D("smoke.png"));
  GFX::TextureManager::Get()->AddTexture("stone", *GFX::LoadTexture2D("stone.png"));

  mainCamera.proj = MakeInfReversedZProjRH(glm::radians(70.0f), GFX::Renderer::Get()->GetWindowAspectRatio(), 0.1f);
  mainCamera.viewInfo.position = { 0, 2, 0 };
  using RMB = GFX::RenderMaskBit;
  GFX::RenderMask mask = RMB::ClearDepthEachFrame |
    RMB::RenderSky |
    RMB::RenderFog |
    RMB::RenderEmitters |
    RMB::RenderObjects |
    RMB::RenderScreenElements |
    RMB::RenderVoxels;
  auto& mainRenderView = scene->GetRenderView("main");
  mainRenderView.camera = &mainCamera;
  mainRenderView.mask = mask;
  mainRenderView.offset = { 0, 0 };
  mainRenderView.size = GFX::Renderer::Get()->GetMainFramebufferDims();

  GFX::TextureCreateInfo cubeMemInfo
  {
    .imageType = GFX::ImageType::TEX_CUBEMAP,
    .format = GFX::Format::R16G16B16A16_FLOAT,
    .extent = { 512, 512, 1 },
    .mipLevels = 1,
    .arrayLayers = 6
  };
  GFX::TextureManager::Get()->AddTexture("probeColor", *GFX::Texture::Create(cubeMemInfo, "Cube Color"));
  cubeMemInfo.format = GFX::Format::D32_FLOAT;
  GFX::TextureManager::Get()->AddTexture("probeDepth", *GFX::Texture::Create(cubeMemInfo, "Cube Depth"));
  const glm::vec3 faceDirs[6] =
  {
    { 1, 0, 0 },
    { -1, 0, 0 },
    { 0, 1, 0 },
    { 0, -1, 0 },
    { 0, 0, 1 },
    { 0, 0, -1 }
  };
  const GFX::Constants::CardinalNames upDirs[6] =
  {
    GFX::Constants::CardinalNames::NegY,
    GFX::Constants::CardinalNames::NegY,
    GFX::Constants::CardinalNames::PosZ,
    GFX::Constants::CardinalNames::NegZ,
    GFX::Constants::CardinalNames::NegY,
    GFX::Constants::CardinalNames::NegY,
  };
  for (uint32_t i = 0; i < 6; i++)
  {
    GFX::TextureViewCreateInfo cubeViewInfo
    {
      .viewType = GFX::ImageType::TEX_2D,
      .format = GFX::Format::R16G16B16A16_FLOAT,
      .minLevel = 0,
      .numLevels = 1,
      .minLayer = i,
      .numLayers = 1
    };
    probeFaces[i].colorView.emplace(*GFX::TextureView::Create(cubeViewInfo, *GFX::TextureManager::Get()->GetTexture("probeColor")));
    cubeViewInfo.format = GFX::Format::D32_FLOAT;
    probeFaces[i].depthView.emplace(*GFX::TextureView::Create(cubeViewInfo, *GFX::TextureManager::Get()->GetTexture("probeDepth")));
    probeFaces[i].fbo.emplace();
    probeFaces[i].fbo->SetAttachment(GFX::Attachment::COLOR_0, *probeFaces[i].colorView, 0);
    probeFaces[i].fbo->SetAttachment(GFX::Attachment::DEPTH, *probeFaces[i].depthView, 0);
    GFX::RenderMask testMask = RMB::ClearDepthEachFrame |
      RMB::RenderSky |
      RMB::RenderFog |
      RMB::RenderVoxels |
      RMB::RenderVoxelsNear |
      RMB::RenderObjects;
    probeFaces[i].camera.proj = MakeInfReversedZProjRH(glm::radians(90.0f), 1.0f, 0.1f);
    probeFaces[i].camera.viewInfo.SetForwardDir(faceDirs[i]);
    probeFaces[i].camera.viewInfo.upDir = upDirs[i];
    GFX::RenderView testView
    {
      .renderTarget = &probeFaces[i].fbo.value(),
      .camera = &probeFaces[i].camera,
      .mask = testMask,
      .offset = { 0, 0 },
      .size = { 512, 512 }
    };
    scene->RegisterRenderView("ProbeFace" + std::to_string(i), testView);
  }


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

    std::vector<MeshID> meshes;
    meshes.push_back(MeshManager::CreateMeshBatched(std::string(ModelDir) + "big_cube.obj", "big_cube"));
    meshes.push_back(MeshManager::CreateMeshBatched(std::string(ModelDir) + "bunny.obj", "bunny"));
    meshes.push_back(MeshManager::CreateMeshBatched(std::string(ModelDir) + "goodSphere.obj", "sphere"));
    meshes.push_back(MeshManager::CreateMeshBatched(std::string(ModelDir) + "teapot.obj", "teapot"));

    Entity testCam = scene->CreateEntity("testCam");
    testCam.AddComponent<Component::Transform>().SetTranslation({ 0, 0, 0 });
    testCam.AddComponent<Component::Model>();
    testCam.AddComponent<Component::BatchedMesh>().handle = MeshManager::GetMeshBatched("big_cube");
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
      parent.AddComponent<Component::BatchedMesh>().handle = MeshManager::GetMeshBatched("big_cube");
      parent.AddComponent<Component::Material>(GFX::MaterialManager::Get()->GetMaterial("batchMaterial"));

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
        entity.AddComponent<Component::BatchedMesh>().handle = MeshManager::GetMeshBatched("big_cube");
        entity.AddComponent<Component::Material>().handle = GFX::MaterialManager::Get()->GetMaterial("batchMaterial");
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
      entity.AddComponent<Component::BatchedMesh>().handle = MeshManager::GetMeshBatched("teapot");
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
      //emitter.handle = ParticleManager::Get().MakeParticleEmitter(1'000'000, "smoke");
      //emitter.data.minLife = 0.5f;
      //emitter.data.maxLife = 1.5f;
      //emitter.data.interval = .000001f;
      //emitter.data.minParticleAccel = { 0, 0, 0 };
      //emitter.data.maxParticleAccel = { 0, 0, 0 };
      //emitter.data.minParticleVelocity = { -1, -10, -1 };
      //emitter.data.maxParticleVelocity = { 1, -9, 1 };
      //emitter.data.minParticleOffset = { -20, 0, -20 };
      //emitter.data.maxParticleOffset = { 20, 20, 20 };
      //emitter.data.minParticleScale = { .005, .02 };
      //emitter.data.maxParticleScale = { .01, .03 };
      //emitter.data.minParticleColor = { .4, .4, 1, .2 };
      //emitter.data.maxParticleColor = { .7, .7, 1, .4 };

      // snow
      emitter.handle = ParticleManager::Get().MakeParticleEmitter(100'000, "smoke");
      emitter.data.minLife = 1;
      emitter.data.maxLife = 2;
      emitter.data.interval = .00002f;
      emitter.data.minParticleAccel = { 0, 0, 0 };
      emitter.data.maxParticleAccel = { 0, 0, 0 };
      emitter.data.minParticleVelocity = { -.5, -1, -.5 };
      emitter.data.maxParticleVelocity = { .1, -.5, .1 };
      emitter.data.minParticleOffset = { -20, 0, -20 };
      emitter.data.maxParticleOffset = { 20, 20, 20 };
      emitter.data.minParticleScale = { .02, .02 };
      emitter.data.maxParticleScale = { .03, .03 };
      emitter.data.minParticleColor = { .6, .6, .6, .6 };
      emitter.data.maxParticleColor = { .7, .7, .7, .8 };

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
  for (int i = 0; i < 6; i++)
  {
    if (!freezeCams)
    {
      probeFaces[i].camera.viewInfo.position = scene->GetEntity("player").GetComponent<Component::Transform>().GetTranslation();
      probeFaces[i].camera.viewInfo.position.y += .65f;
    }
    ImGui::Image((ImTextureID)probeFaces[i].colorView->GetAPIHandle(), { 128, 128 });
  }
  //ImGui::SliderFloat("Pitch", &vi.pitch, -glm::pi<float>() / 2, glm::pi<float>() / 2);
  //ImGui::SliderFloat("Yaw", &vi.yaw, 0, 6.28);
  //ImGui::SliderFloat3("Pos", glm::value_ptr(vi.position), 0, 10);
  auto renderViews = scene->GetRenderViews();
  voxelManager->Draw();
  GFX::Renderer::Get()->DrawAxisIndicator(renderViews);
}

int main()
{
  engine::Core::InitLogging();

  Application::SetStartCallback(OnStart);
  Application::SetUpdateCallback(OnUpdate);
  Application::SetDrawOpaqueCallback(OnDraw);

  Application::Start();
  voxelManager.reset();
  Application::Shutdown();

  return 0;
}