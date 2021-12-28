#pragma once
#include <map>
#include <span>

#include "api/DynamicBuffer.h"
#include "api/Indirect.h"
#include "RenderView.h"
#include "Camera.h"

namespace Component
{
  struct Transform;
  struct BatchedMesh;
  struct Material;
  struct ParticleEmitter;
  struct Model;
}

struct GLFWwindow;

namespace GFX
{
  namespace Renderer
  {
    GLFWwindow* const* Init();

    // big boy drawing functions
    void BeginObjects(size_t maxDraws);
    void SubmitObject(const Component::Model& model, const Component::BatchedMesh& mesh, const Component::Material& mat);
    void RenderObjects(std::span<RenderView*> renderViews);

    void BeginEmitters(size_t maxDraws);
    void SubmitEmitter(const Component::ParticleEmitter& emitter, const Component::Transform& model);
    void RenderEmitters(std::span<RenderView*> renderViews);

    void DrawFog(std::span<RenderView*> renderViews, bool earlyFogPass);

    // generic drawing functions (TODO: move)
    void DrawAxisIndicator(std::span<RenderView*> renderViews);
    void DrawSky(std::span<RenderView*> renderViews);

    void StartFrame();
    void ClearFramebuffers(std::span<RenderView*> renderViews);
    void DrawReflections();
    void ApplyAntialiasing();
    void WriteSwapchain();
    void Bloom();
    void ApplyTonemap(float dt);

    // feel free to modify the camera and/or the render mask, but not the render info of the returned RenderView
    [[nodiscard]] RenderView* GetMainRenderView();
    [[nodiscard]] RenderView* GetProbeRenderView(size_t index);
    [[nodiscard]] Extent2D GetWindowDimensions();

    [[nodiscard]] GFX::DynamicBuffer<>* GetVertexBuffer();

    [[nodiscard]] GFX::DynamicBuffer<>* GetIndexBuffer();

    [[nodiscard]] std::map<uint32_t, DrawElementsIndirectCommand>* GetMeshBufferInfos();

    [[nodiscard]] float GetWindowAspectRatio();

    [[nodiscard]] bool GetIsFullscreen();

    void SetFramebufferSize(uint32_t width, uint32_t height);
    void SetRenderingScale(float scale);
    void SetReflectionsRenderScale(float scale);
    GLFWwindow* CreateWindow(bool fullscreen);
    void InitFramebuffers();
    void InitReflectionFramebuffer();

    bool QueryOpenGLExtensionStatus(std::string_view extensionName);
    const std::vector<std::string>& GetAllOpenGLExtensions();

    void SetProbePosition(glm::vec3 worldPos);
    void SetProbeRenderMask(RenderMask mask);
    void SetUpdateProbes(bool b); // if true, probes will be updated (expensive)

    static inline constexpr float REFLECTION_MODE_PARALLAX_CUBE_THRESHOLD = 2.0f;
    static inline constexpr float REFLECTION_MODE_CUBE_THRESHOLD = 1.0f;
  };

  void SetFramebufferDrawBuffersAuto(Framebuffer& framebuffer, const RenderInfo& renderInfo, size_t maxCount);
}