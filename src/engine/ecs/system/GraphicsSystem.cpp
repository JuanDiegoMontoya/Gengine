#include "../../PCH.h"
#include "../../Scene.h"
#include <glad/glad.h>
#include "GraphicsSystem.h"
#include <engine/Input.h>
#include <engine/Application.h>
#include <engine/Camera.h>
#include <engine/gfx/Renderer.h>
#include <engine/gfx/Mesh.h>
#include <engine/gfx/Fence.h>
#include <engine/core/Statistics.h>
#include <engine/core/StatMacros.h>
#include <execution>
#include <glm/gtx/norm.hpp>

#include <imgui/imgui.h>

// TODO: TEMP GARBAGE
#include <engine/Context.h>

#include "../component/Rendering.h"
#include "../component/Camera.h"
#include "../component/ParticleEmitter.h"

DECLARE_FLOAT_STAT(DrawTransparentGeom, GPU)

void GraphicsSystem::Init()
{
  window = init_glfw_context();
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  CameraSystem::Init();
  GFX::Renderer::Get()->Init();
}

void GraphicsSystem::Shutdown()
{
  glfwDestroyWindow(window);
}

void GraphicsSystem::StartFrame()
{
  GFX::Renderer::Get()->StartFrame();

  ImGui::Begin("Graphics");
  ImGui::Text("Loaded Meshes");
  for (const auto& [id, info] : MeshManager::handleMap_)
  {
    ImGui::Text("%s, ID: %u", id.data(), id.value());
  }
  ImGui::Separator();
  ImGui::Text("Loaded Materials");
  for (const auto& [id, info] : GFX::MaterialManager::Get()->materials_)
  {
    ImGui::Text("ID: %u", id);
  }
  ImGui::End();

  const auto& camList = CameraSystem::GetCameraList();
  for (Component::Camera* camera : camList)
  {
    if (!camera->skyboxTexture)
    {
      continue;
    }
    CameraSystem::ActiveCamera = camera;
    CameraSystem::Update();
    GFX::Renderer::Get()->DrawSkybox();
  }
}

void GraphicsSystem::DrawOpaque(Scene& scene)
{
  const auto& camList = CameraSystem::GetCameraList();
  for (Component::Camera* camera : camList)
  {
    CameraSystem::ActiveCamera = camera;
    CameraSystem::Update();

    // draw batched objects in the scene
    using namespace Component;
    auto group = scene.GetRegistry().group<BatchedMesh>(entt::get<Model, Material>);
    GFX::Renderer::Get()->BeginBatch(group.size());
    std::for_each(std::execution::par, group.begin(), group.end(),
      [&group](entt::entity entity)
      {
        auto [mesh, model, material] = group.get<BatchedMesh, Model, Material>(entity);
        if ((CameraSystem::ActiveCamera->cullingMask & mesh.renderFlag) != mesh.renderFlag) // Mesh not set to be culled
        {
          GFX::Renderer::Get()->Submit(model, mesh, material);
        }
      });

    GFX::Renderer::Get()->RenderBatch();
  }
}

void GraphicsSystem::DrawTransparent(Scene& scene)
{
  //static GFX::TimerQueryAsync timerr(5);
  //GFX::TimerScoped tz(timerr);
  //auto result = timerr.Elapsed_ns();
  //if (result)
  //{
  //  printf("DrawTransparentGeomA %f ms\n", (double)*result / 1000000.0);
  //}
  //else
  //{
  //  //printf("No emitter render time stats\n");
  //}

  MEASURE_GPU_TIMER_STAT(DrawTransparentGeom);

  const auto& camList = CameraSystem::GetCameraList();
  for (Component::Camera* camera : camList)
  {
    CameraSystem::ActiveCamera = camera;

    // draw particles from back to front
    using namespace Component;
    auto view = scene.GetRegistry().view<ParticleEmitter, Transform>();
    auto compare = [&camera](const auto& p1, const auto& p2)
    {
      if (p1.second->GetTranslation() != p2.second->GetTranslation())
      {
        auto len = glm::length2(p1.second->GetTranslation() - camera->GetWorldPos()) -
          glm::length2(p2.second->GetTranslation() - camera->GetWorldPos());
        if (glm::abs(len) > .001f)
        {
          return len > 0.0f;
        }
      }
      return p1.first < p2.first;
    };
    std::vector<std::pair<ParticleEmitter*, Transform*>> emitters;
    for (auto entity : view)
    {
      auto [emitter, transform] = view.get<ParticleEmitter, Transform>(entity);
      if ((CameraSystem::ActiveCamera->cullingMask & emitter.renderFlag) != emitter.renderFlag) // Emitter not set to be culled
      {
        emitters.push_back(std::pair<ParticleEmitter*, Transform*>(&emitter, &transform));
      }
    }

    std::sort(emitters.begin(), emitters.end(), compare);
    GFX::Renderer::Get()->BeginRenderParticleEmitter();
    for (const auto& [emitter, transform] : emitters)
    {
      GFX::Renderer::Get()->RenderParticleEmitter(*emitter, *transform);
    }
  }
}

void GraphicsSystem::EndFrame(Timestep timestep)
{
  GFX::Renderer::Get()->EndFrame(timestep.dt_effective);
}

void GraphicsSystem::SwapBuffers()
{
  glfwSwapBuffers(window);
}
