#pragma once
#include <vector>
#include <map>
#include <span>

#include "MeshUtils.h"
#include "Material.h"
#include "DynamicBuffer.h"
#include "Indirect.h"
#include "Texture.h"
#include "Framebuffer.h"
#include "RenderView.h"

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
  class Renderer
  {
  public:
    static [[nodiscard]] Renderer* Get();

    Renderer(const Renderer&) = delete;
    Renderer(Renderer&&) = delete;
    Renderer& operator=(const Renderer&) = delete;
    Renderer& operator=(Renderer&&) = delete;

    GLFWwindow* const* Init();

    // big boy drawing functions
    void BeginObjects(size_t maxDraws);
    void SubmitObject(const Component::Model& model, const Component::BatchedMesh& mesh, const Component::Material& mat);
    void RenderObjects(std::span<RenderView> renderViews);

    void BeginEmitters(size_t maxDraws);
    void SubmitEmitter(const Component::ParticleEmitter& emitter, const Component::Transform& model);
    void RenderEmitters(std::span<RenderView> renderViews);
 
    void DrawFog(std::span<RenderView> renderViews);

    // generic drawing functions (TODO: move)
    void DrawAxisIndicator(std::span<RenderView> renderViews);
    void DrawSkybox(std::span<RenderView> renderViews);

    void StartFrame();
    void ClearFramebuffers(std::span<RenderView> renderViews);
    void EndFrame(float dt);

    // returns the framebuffer which is displayed on the screen
    // do not modify this framebuffer
    [[nodiscard]] Framebuffer* GetMainFramebuffer() { return &hdrFbo.value(); }
    [[nodiscard]] Extent2D GetMainFramebufferDims() const { return { renderWidth, renderHeight }; }

    [[nodiscard]] GFX::DynamicBuffer<>* GetVertexBuffer()
    {
      return vertexBuffer.get();
    }

    [[nodiscard]] GFX::DynamicBuffer<>* GetIndexBuffer()
    {
      return indexBuffer.get();
    }

    [[nodiscard]] auto* GetMeshBufferInfos()
    {
      return &meshBufferInfo;
    }

    [[nodiscard]] float GetWindowAspectRatio() const
    {
      return static_cast<float>(windowWidth) / static_cast<float>(windowHeight);
    }

    [[nodiscard]] bool GetIsFullscreen() const { return isFullscreen; }

    void SetFramebufferSize(uint32_t width, uint32_t height);
    void SetRenderingScale(float scale);
    GLFWwindow* CreateWindow(bool fullscreen);
    void InitFramebuffers();

  private:
    Renderer() {};
    ~Renderer() {};

    GLFWwindow* InitContext();
    void InitVertexBuffers();
    void InitVertexLayouts();
    void CompileShaders();
    void InitTextures();

    GLFWwindow* window_{};
    bool isFullscreen{ false };

    // resets the GL state to something predictable
    void GL_ResetState();

    // particle rendering
    struct EmitterDrawCommand
    {
      const Component::ParticleEmitter* emitter;
      glm::mat4 modelUniform;
    };
    std::atomic_uint32_t emitterDrawIndex = 0;
    std::vector<EmitterDrawCommand> emitterDrawCommands;

    // std140
    struct UniformData
    {
      glm::mat4 model;
    };
    void RenderBatchHelper(std::span<RenderView> renderViews, MaterialID material, const std::vector<UniformData>& uniformBuffer);

    // batched+instanced rendering stuff (ONE MATERIAL SUPPORTED ATM)
    std::unique_ptr<GFX::DynamicBuffer<>> vertexBuffer;
    std::unique_ptr<GFX::DynamicBuffer<>> indexBuffer;

    // per-vertex layout
    uint32_t batchVAO{};

    // maps handles to VERTEX and INDEX information in the respective dynamic buffers
    // used to retrieve important offset and size info for meshes
    std::map<uint32_t, DrawElementsIndirectCommand> meshBufferInfo;

    struct BatchDrawCommand
    {
      MeshID mesh;
      MaterialID material;
      glm::mat4 modelUniform;
    };
    std::vector<BatchDrawCommand> userCommands;
    std::atomic_uint32_t cmdIndex{ 0 };

    uint32_t emptyVao{};

    uint32_t axisVao{};
    std::optional<StaticBuffer> axisVbo;

    std::optional<TextureSampler> defaultSampler;

    // HDR inverse-z framebuffer stuff
    std::optional<Framebuffer> ldrFbo;
    std::optional<Texture> ldrColorTexMemory;
    std::optional<TextureView> ldrColorTexView;
    //uint32_t ldrColorTex{};
    uint32_t windowWidth{ 1 };
    uint32_t windowHeight{ 1 };
    uint32_t renderWidth{ 1 };
    uint32_t renderHeight{ 1 };
    float renderScale{ 1.0f }; // 1.0 means render resolution will match window
    uint32_t GetRenderWidth() const { return renderWidth; }
    uint32_t GetRenderHeight() const { return renderHeight; }
    std::optional<Framebuffer> hdrFbo;
    std::optional<Texture> hdrColorTexMemory;
    std::optional<Texture> hdrDepthTexMemory;
    std::optional<TextureView> hdrColorTexView;
    std::optional<TextureView> hdrDepthTexView;

    struct Environment
    {
      std::optional<Texture> skyboxMemory;
      std::optional<TextureView> skyboxView;
      std::optional<TextureSampler> skyboxSampler;
    }env;

    struct FogParams
    {
      glm::vec3 albedo{ 1.0 };
      float u_a = 0.005f;
      float u_b = 10000.0f;
      float u_heightOffset = -40.0f;
      float u_fog2Density = 0.005f;
      float u_beer = 1.0f;
      float u_powder = 1.0f;
    }fog;

    struct TonemapperParams
    {
      float exposure = 1.0f;
      float minExposure = 0.1f;
      float maxExposure = 10.0f;
      float targetLuminance = .22f;
      float adjustmentSpeed = 0.5f;
      bool gammaCorrection = true;
      std::unique_ptr<GFX::StaticBuffer> exposureBuffer;
      std::unique_ptr<GFX::StaticBuffer> histogramBuffer;
      const int NUM_BUCKETS = 128;
      std::optional<GFX::TextureView> blueNoiseView;
      std::optional<GFX::TextureSampler> blueNoiseSampler;
      bool tonemapDither = true;
    }tonemap;

    struct FXAAParams
    {
      bool enabled{ true };
      float contrastThreshold = 0.0312;
      float relativeThreshold = 0.125;
      float pixelBlendStrength = 0.2;
      float edgeBlendStrength = 1.0;
    }fxaa;
  };
}