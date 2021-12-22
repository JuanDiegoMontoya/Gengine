#pragma once
#include <vector>
#include <map>
#include <span>
#include <array>

#include "MeshUtils.h"
#include "Material.h"
#include "api/DynamicBuffer.h"
#include "api/Indirect.h"
#include "api/Texture.h"
#include "api/Framebuffer.h"
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
    [[nodiscard]] RenderView* GetMainRenderView() { return &gBuffer.renderView; }
    [[nodiscard]] RenderView* GetProbeRenderView(size_t index) { ASSERT(index < 6); return &probeData.renderViews[index]; }
    [[nodiscard]] Extent2D GetWindowDimensions() const { return { renderWidth, renderHeight }; }

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
    void SetReflectionsRenderScale(float scale);
    GLFWwindow* CreateWindow(bool fullscreen);
    void InitFramebuffers();
    void InitReflectionFramebuffer();

    bool QueryOpenGLExtensionStatus(std::string_view extensionName);
    const std::vector<std::string>& GetAllOpenGLExtensions();

    void SetProbePosition(glm::vec3 worldPos);
    void SetProbeRenderMask(RenderMask mask);

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

    std::vector<std::string> openglExtensions;

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
    void RenderBatchHelper(std::span<RenderView*> renderViews, MaterialID material, const std::vector<UniformData>& uniformBuffer);

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
    std::optional<Buffer> axisVbo;

    std::optional<TextureSampler> defaultSampler;
    std::optional<TextureSampler> nearestSampler;
    std::optional<TextureSampler> linearMipmapNearestSampler;

    std::optional<Texture> ldrColorTexMemory;
    std::optional<TextureView> ldrColorTexView;
    std::optional<Texture> postAATexMemory;
    std::optional<TextureView> postAATexView;
    uint32_t windowWidth{ 1 };
    uint32_t windowHeight{ 1 };
    uint32_t renderWidth{ 1 };
    uint32_t renderHeight{ 1 };
    float renderScale{ 1.0f }; // 1.0 means render resolution will match window
    uint32_t GetRenderWidth() const { return renderWidth; }
    uint32_t GetRenderHeight() const { return renderHeight; }

    struct GBuffer
    {
      std::optional<Framebuffer> fbo;
      RenderView renderView{};
      Camera camera{};
      std::optional<Texture> colorTexMemory;
      std::optional<Texture> normalTexMemory;
      std::optional<Texture> depthTexMemory;
      std::optional<Texture> PBRTexMemory;
      std::optional<TextureView> colorTexView;
      std::optional<TextureView> normalTexView;
      std::optional<TextureView> depthTexView;
      std::optional<TextureView> PBRTexView;
    }gBuffer;

    std::optional<TextureView> blueNoiseRView;

    std::optional<TextureView> blueNoiseBigView;

    struct Reflections_t
    {
      static inline constexpr float MODE_PARALLAX_CUBE_THRESHOLD = 2.0f;
      static inline constexpr float MODE_CUBE_THRESHOLD = 1.0f;
      std::optional<Framebuffer> fbo;
      std::optional<Texture> texMemory[2];
      std::optional<TextureView> texView[2];
      std::optional<TextureSampler> scratchSampler;
      std::optional<TextureSampler> scratchSampler2;
      float renderScale{};
      Extent2D fboSize{};

      struct
      {
        std::array<float, 5> kernel = { 0.0625f, 0.25f, 0.375f, 0.25f, 0.0625f };
        std::array<float, 5> offsets = { -2.0f, -1.0f, 0.0f, 1.0f, 2.0f };
        uint32_t num_passes{ 1 };
        float n_phi{ 1 };
        float p_phi{ 1 };
        float step_width{ 1.0f };
      }atrous;
    }reflect;

    struct ProbeData_t
    {
      std::array<RenderView, 6> renderViews;
      std::array<Camera, 6> cameras;
      std::array<std::optional<TextureView>, 6> colorViews;
      std::array<std::optional<TextureView>, 6> depthViews;
      std::array<std::optional<TextureView>, 6> distanceViews;
      std::optional<Texture> colorCube;
      std::optional<Texture> depthCube;
      std::optional<Texture> distanceCube;
      std::optional<TextureView> colorCubeView;
      std::optional<TextureView> distanceCubeView;
      Format colorFormat = Format::R11G11B10_FLOAT;
      Format depthFormat = Format::D16_UNORM;
      Format distanceFormat = Format::R16_FLOAT;
      Extent2D imageSize{ 512, 512 };
    }probeData;

    struct Composited_t
    {
      // post-shading
      std::optional<Framebuffer> fbo;
    }composited;

    struct Environment_t
    {
      std::optional<Texture> skyboxMemory;
      std::optional<TextureView> skyboxView;
      std::optional<TextureSampler> skyboxSampler;
    }env;

    struct FogParams_t
    {
      glm::vec3 albedo{ 1.0 };
      float u_a = 0.005f;
      float u_b = 10000.0f;
      float u_heightOffset = -40.0f;
      float u_fog2Density = 0.005f;
      float u_beer = 1.0f;
      float u_powder = 1.0f;
    }fog;

    struct TonemapperParams_t
    {
      float exposure = 1.0f;
      float minExposure = 0.1f;
      float maxExposure = 10.0f;
      float targetLuminance = .22f;
      float adjustmentSpeed = 0.5f;
      bool gammaCorrection = true;
      std::optional<GFX::Buffer> exposureBuffer;
      std::optional<GFX::Buffer> histogramBuffer;
      const int NUM_BUCKETS = 128;
      std::optional<GFX::TextureView> blueNoiseView;
      std::optional<GFX::TextureSampler> blueNoiseSampler;
      bool tonemapDither = true;
    }tonemap;

    struct Bloom_t
    {
      bool enabled{ true };
      float strength{ 1.0f / 64.0f };
      float width{ 1.0f };
      uint32_t passes{ 6 };
      std::optional<Texture> scratchTex;
      std::optional<TextureView> scratchTexView;
      std::optional<TextureSampler> scratchSampler;
    }bloom;

    struct FXAAParams_t
    {
      bool enabled{ true };
      float contrastThreshold = 0.0312;
      float relativeThreshold = 0.125;
      float pixelBlendStrength = 0.2;
      float edgeBlendStrength = 1.0;
      std::optional<TextureSampler> scratchSampler;
    }fxaa;
  };

  void SetFramebufferDrawBuffersAuto(Framebuffer& framebuffer, const RenderInfo& renderInfo, size_t maxCount);
}