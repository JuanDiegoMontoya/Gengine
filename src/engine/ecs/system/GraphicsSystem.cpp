#include "../../PCH.h"
#include "../../Scene.h"
#include <glad/glad.h>
#include "GraphicsSystem.h"
#include <engine/Input.h>
#include <engine/Application.h>
#include <engine/gfx/Renderer.h>
#include <engine/gfx/Mesh.h>
#include <engine/gfx/Fence.h>
#include <engine/gfx/RenderView.h>
#include <engine/gfx/Camera.h>
#include <engine/core/Statistics.h>
#include <engine/core/StatMacros.h>
#include <execution>
#include <glm/gtx/norm.hpp>
#include <entt/entity/registry.hpp>

#include <imgui/imgui.h>

#include <engine/ecs/component/Transform.h>
#include "../component/Rendering.h"
#include "../component/ParticleEmitter.h"

DECLARE_FLOAT_STAT(DrawTransparent_GPU, GPU)
DECLARE_FLOAT_STAT(DrawTransparent_CPU, CPU)
DECLARE_FLOAT_STAT(DrawOpaque_GPU, GPU)
DECLARE_FLOAT_STAT(DrawOpaque_CPU, CPU)
DECLARE_FLOAT_STAT(SwapBuffers_CPU, CPU)

void GraphicsSystem::Init()
{
  window = GFX::Renderer::Get()->Init();
}

void GraphicsSystem::Shutdown()
{
  glfwTerminate();
}

void GraphicsSystem::StartFrame(Scene& scene)
{
  auto renderViews = scene.GetRenderViews();
  GFX::Renderer::Get()->StartFrame();
  GFX::Renderer::Get()->ClearFramebuffers(renderViews);

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
}

void GraphicsSystem::DrawOpaque(Scene& scene)
{
  MEASURE_GPU_TIMER_STAT(DrawOpaque_GPU);
  MEASURE_CPU_TIMER_STAT(DrawOpaque_CPU);

  auto renderViews = scene.GetRenderViews();

  // draw batched objects in the scene
  using namespace Component;
  auto group = scene.GetRegistry().group<BatchedMesh>(entt::get<Model, Material>);
  GFX::Renderer::Get()->BeginObjects(group.size());

  std::for_each(std::execution::par, group.begin(), group.end(),
    [&group](entt::entity entity)
    {
      auto [mesh, model, material] = group.get<BatchedMesh, Model, Material>(entity);
      GFX::Renderer::Get()->SubmitObject(model, mesh, material);
    });

  GFX::Renderer::Get()->RenderObjects(renderViews);
}

void GraphicsSystem::DrawSky(Scene& scene)
{
  auto renderViews = scene.GetRenderViews();
  GFX::Renderer::Get()->DrawSkybox(renderViews);
}

void GraphicsSystem::DrawTransparent(Scene& scene)
{
  MEASURE_GPU_TIMER_STAT(DrawTransparent_GPU);
  MEASURE_CPU_TIMER_STAT(DrawTransparent_CPU);

  auto renderViews = scene.GetRenderViews();

  using namespace Component;
  auto view = scene.GetRegistry().view<ParticleEmitter, Transform>();
  GFX::Renderer::Get()->BeginEmitters(view.size_hint()); // multi-component views only know the maximum possible size

  std::for_each(std::execution::par, view.begin(), view.end(),
    [&view](entt::entity entity)
    {
      auto [emitter, transform] = view.get<ParticleEmitter, Transform>(entity);
      GFX::Renderer::Get()->SubmitEmitter(emitter, transform);
    });

  GFX::Renderer::Get()->RenderEmitters(renderViews);
}

void GraphicsSystem::DrawFog(Scene& scene)
{
  auto renderViews = scene.GetRenderViews();
  GFX::Renderer::Get()->DrawFog(renderViews);
}

void GraphicsSystem::EndFrame(Scene& scene, Timestep timestep)
{
  auto renderViews = scene.GetRenderViews();
  GFX::Renderer::Get()->EndFrame(timestep.dt_effective);
}

void GraphicsSystem::SwapBuffers()
{
  MEASURE_CPU_TIMER_STAT(SwapBuffers_CPU);
  glfwSwapBuffers(*window);
}
