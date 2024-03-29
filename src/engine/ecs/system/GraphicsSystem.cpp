#include "../../PCH.h"
#include "../../Scene.h"
#include <glad/glad.h>
#include "GraphicsSystem.h"
#include <engine/Input.h>
#include <engine/Application.h>
#include <engine/gfx/Renderer.h>
#include <engine/gfx/Mesh.h>
#include <engine/gfx/api/Fence.h>
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
  window = GFX::Renderer::Init();
}

void GraphicsSystem::Shutdown()
{
  glfwTerminate();
}

void GraphicsSystem::StartFrame(Scene& scene)
{
  auto renderViews = scene.GetRenderViews();
  GFX::Renderer::StartFrame();
  GFX::Renderer::ClearFramebuffers(renderViews);
}

void GraphicsSystem::DrawOpaque(Scene& scene)
{
  MEASURE_GPU_TIMER_STAT(DrawOpaque_GPU);
  MEASURE_CPU_TIMER_STAT(DrawOpaque_CPU);

  auto renderViews = scene.GetRenderViews();

  // draw batched objects in the scene
  using namespace Component;
  auto group = scene.GetRegistry().group<BatchedMesh>(entt::get<Model, Material>);
  GFX::Renderer::BeginObjects(group.size());

  std::for_each(std::execution::par, group.begin(), group.end(),
    [&group](entt::entity entity)
    {
      auto [mesh, model, material] = group.get<BatchedMesh, Model, Material>(entity);
      GFX::Renderer::SubmitObject(model, mesh, material);
    });

  GFX::Renderer::RenderObjects(renderViews);
}

void GraphicsSystem::DrawSky(Scene& scene)
{
  auto renderViews = scene.GetRenderViews();
  GFX::Renderer::DrawSky(renderViews);
}

void GraphicsSystem::DrawShading(Scene& scene)
{
  auto renderViews = scene.GetRenderViews();
  GFX::Renderer::DrawReflections();
}

void GraphicsSystem::DrawFog(Scene& scene)
{
  auto renderViews = scene.GetRenderViews();
  GFX::Renderer::DrawFog(renderViews, false);
}

void GraphicsSystem::DrawEarlyFog(Scene& scene)
{
  auto renderViews = scene.GetRenderViews();
  GFX::Renderer::DrawFog(renderViews, true);
}

void GraphicsSystem::DrawTransparent(Scene& scene)
{
  MEASURE_GPU_TIMER_STAT(DrawTransparent_GPU);
  MEASURE_CPU_TIMER_STAT(DrawTransparent_CPU);

  auto renderViews = scene.GetRenderViews();

  using namespace Component;
  auto view = scene.GetRegistry().view<ParticleEmitter, Transform>();
  GFX::Renderer::BeginEmitters(view.size_hint()); // multi-component views only know the maximum possible size

  std::for_each(std::execution::par, view.begin(), view.end(),
    [&view](entt::entity entity)
    {
      auto [emitter, transform] = view.get<ParticleEmitter, Transform>(entity);
      GFX::Renderer::SubmitEmitter(emitter, transform);
    });

  GFX::Renderer::RenderEmitters(renderViews);
}

void GraphicsSystem::Bloom()
{
  GFX::Renderer::Bloom();
}

void GraphicsSystem::EndFrame(Timestep timestep)
{
  GFX::Renderer::ApplyTonemap(timestep.dt_effective);
  GFX::Renderer::ApplyAntialiasing();
  GFX::Renderer::WriteSwapchain();
}

void GraphicsSystem::SwapBuffers()
{
  MEASURE_CPU_TIMER_STAT(SwapBuffers_CPU);
  glfwSwapBuffers(*window);
}
