#pragma once
#include <vector>
#include <map>

#include <engine/gfx/MeshUtils.h>
#include <engine/gfx/Material.h>
#include <engine/gfx/DynamicBuffer.h>
#include <engine/gfx/Indirect.h>
#include "Texture.h"

namespace Component
{
  struct Transform;
  struct BatchedMesh;
  struct Material;
  struct ParticleEmitter;
}

namespace GFX
{
  class Renderer
  {
  public:
    static void Init();
    static void CompileShaders();

    static void BeginBatch(size_t size);
    static void Submit(const Component::Transform& model, const Component::BatchedMesh& mesh, const Component::Material& mat);
    static void RenderBatch();
    static void BeginRenderParticleEmitter();
    static void RenderParticleEmitter(const Component::ParticleEmitter& emitter, const Component::Transform& model);

    // generic drawing functions (TODO: move)
    static void DrawAxisIndicator();

    static void DrawSkybox();

    static void StartFrame();
    static void EndFrame(float dt);

  private:
    friend class MeshManager;
    friend class ParticleSystem;
    friend class GraphicsSystem;

    // std140
    struct UniformData
    {
      glm::mat4 model;
    };
    static void RenderBatchHelper(MaterialID material, const std::vector<UniformData>& uniformBuffer);

    // batched+instanced rendering stuff (ONE MATERIAL SUPPORTED ATM)
    static inline std::unique_ptr<GFX::DynamicBuffer<>> vertexBuffer;
    static inline std::unique_ptr<GFX::DynamicBuffer<>> indexBuffer;

    // per-vertex layout
    static inline uint32_t batchVAO;

    // maps handles to VERTEX and INDEX information in the respective dynamic buffers
    // used to retrieve important offset and size info for meshes
    using DBaT = GFX::DynamicBuffer<>::allocationData<>;
    static inline std::map<uint32_t, DrawElementsIndirectCommand> meshBufferInfo;

    struct BatchDrawCommand
    {
      MeshID mesh;
      MaterialID material;
      glm::mat4 modelUniform;
    };
    static inline std::vector<BatchDrawCommand> userCommands;
    static inline std::atomic_uint32_t cmdIndex{ 0 };

    static inline uint32_t emptyVao{};

    // HDR inverse-z framebuffer stuff
    static inline uint32_t hdrFbo{};
    static inline uint32_t ldrFbo{};
    static inline uint32_t ldrColorTex{};
    static inline int windowWidth = 1920, windowHeight = 1017;
    static inline int fboWidth = 1920, fboHeight = 1017;
    //static inline int windowWidth = 10, windowHeight = 10;
    //static inline int fboWidth = 10, fboHeight = 10;
    static inline uint32_t hdrColorTex;
    static inline uint32_t hdrDepthTex;

    struct
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
    }static inline tonemap;

    struct
    {
      bool enabled{ true };
      float contrastThreshold = 0.0312;
      float relativeThreshold = 0.125;
      float pixelBlendStrength = 0.2;
      float edgeBlendStrength = 1.0;
    }static inline fxaa;
  };
}