#include "EnginePCH.h"
#include "GraphicsSystem.h"
#include "Scene.h"
#include <CoreEngine/GraphicsIncludes.h>
#include <CoreEngine/Input.h>
#include <CoreEngine/Application.h>
#include <CoreEngine/Camera.h>
#include <CoreEngine/Components.h>
#include <CoreEngine/Renderer.h>
#include <CoreEngine/Mesh.h>
#include <execution>

#include <imgui/imgui.h>

// TODO: TEMP GARBAGE
#include <CoreEngine/Context.h>

void GraphicsSystem::Init()
{
  //cam = new Camera();
  //Camera::ActiveCamera = cam;
  window = init_glfw_context();
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);


  GLuint color, depth;
  glGenTextures(1, &color);
  glBindTexture(GL_TEXTURE_2D, color);
  glTexStorage2D(GL_TEXTURE_2D, 1, GL_SRGB8_ALPHA8, fboWidth, fboHeight);
  glBindTexture(GL_TEXTURE_2D, 0);

  glGenTextures(1, &depth);
  glBindTexture(GL_TEXTURE_2D, depth);
  glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT32F, fboWidth, fboHeight);
  glBindTexture(GL_TEXTURE_2D, 0);

  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth, 0);
  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (status != GL_FRAMEBUFFER_COMPLETE)
  {
    fprintf(stderr, "glCheckFramebufferStatus: %x\n", status);
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  CameraSystem::Init();
  Renderer::Init();
}

void GraphicsSystem::Shutdown()
{
  glfwDestroyWindow(window);
}

void GraphicsSystem::StartFrame()
{
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  glClearDepth(0.0f);
  auto cc = glm::vec3(.529f, .808f, .922f);
  glClearColor(cc.r, cc.g, cc.b, 1.f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_GREATER);
  glEnable(GL_FRAMEBUFFER_SRGB); // gamma correction
}

void GraphicsSystem::DrawOpaque(Scene& scene, float dt)
{
  const auto& camList = CameraSystem::GetCameraList();
  for (Components::Camera* camera : camList)
  {
    CameraSystem::ActiveCamera = camera;
    CameraSystem::Update(dt);

    // draw batched objects in the scene
    using namespace Components;
    auto group = scene.GetRegistry().group<BatchedMesh>(entt::get<Transform, Material>);
    Renderer::BeginBatch(group.size());
    std::for_each(std::execution::par_unseq, group.begin(), group.end(),
      [&group](entt::entity entity)
      {
        auto [mesh, transform, material] = group.get<BatchedMesh, Transform, Material>(entity);
        Renderer::Submit(transform, mesh, material);
      });

      Renderer::RenderBatch();
  }

}

void GraphicsSystem::DrawTransparent(Scene& scene, float dt)
{
  const auto& camList = CameraSystem::GetCameraList();
  for (Components::Camera* camera : camList)
  {
    CameraSystem::ActiveCamera = camera;

    // draw particles
    using namespace Components;
    auto view = scene.GetRegistry().view<ParticleEmitter, Transform>();
    for (auto entity : view)
    {
      auto [emitter, transform] = view.get<ParticleEmitter, Transform>(entity);
      Renderer::RenderParticleEmitter(emitter, transform);
    }
  }
}

void GraphicsSystem::EndFrame()
{
  glDisable(GL_FRAMEBUFFER_SRGB);
  glDepthFunc(GL_LESS);
  glDisable(GL_DEPTH_TEST);
  
  glBlitNamedFramebuffer(fbo, 0,
    0, 0, fboWidth, fboHeight,
    0, 0, windowWidth, windowHeight,
    GL_COLOR_BUFFER_BIT, GL_LINEAR);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glfwSwapBuffers(window);
}
