#pragma once
#include <vector>
#include <map>

#include <engine/gfx/MeshUtils.h>
#include <engine/gfx/Material.h>
#include <engine/gfx/DynamicBuffer.h>
#include <engine/gfx/Indirect.h>
#include "Texture.h"
#include "Framebuffer.h"

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
    void CompileShaders();

    void BeginBatch(size_t size);
    void Submit(const Component::Model& model, const Component::BatchedMesh& mesh, const Component::Material& mat);
    void RenderBatch();
    void BeginRenderParticleEmitter();
    void RenderParticleEmitter(const Component::ParticleEmitter& emitter, const Component::Transform& model);

    // generic drawing functions (TODO: move)
    void DrawAxisIndicator();

    void DrawSkybox();

    void StartFrame();
    void EndFrame(float dt);

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

    bool GetIsFullscreen() const { return isFullscreen; }

    void SetFramebufferSize(uint32_t width, uint32_t height);
    void SetRenderingScale(float scale);
    GLFWwindow* CreateWindow(bool fullscreen);
    void InitFramebuffers();

  private:
    Renderer() {};
    ~Renderer() {};

    void InitVertexLayouts();
    GLFWwindow* InitContext();

    GLFWwindow* window_{};
    bool isFullscreen{ false };

    // resets the GL state to something predictable
    void GL_ResetState();

    // std140
    struct UniformData
    {
      glm::mat4 model;
    };
    void RenderBatchHelper(MaterialID material, const std::vector<UniformData>& uniformBuffer);

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

    struct FogParams
    {
      std::optional<Framebuffer> framebuffer;
      std::optional<Texture> texMemory;
      std::optional<TextureView> texView;
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