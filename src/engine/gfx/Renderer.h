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
  static inline uint32_t fbo{};
  static inline uint32_t ldrFbo{};
  static inline uint32_t ldrColor{};
  static inline int windowWidth = 1920, windowHeight = 1017;
  static inline int fboWidth = 1920, fboHeight = 1017;
  //static inline int windowWidth = 10, windowHeight = 10;
  //static inline int fboWidth = 10, fboHeight = 10;
  static inline uint32_t color;
  static inline uint32_t depth;
  static inline float exposure = 1.0f;
  static inline float minExposure = 0.1f;
  static inline float maxExposure = 10.0f;
  static inline float targetLuminance = .22f;
  static inline float adjustmentSpeed = 0.5f;
  static inline bool gammaCorrection = true;
  static inline std::unique_ptr<GFX::StaticBuffer> exposureBuffer;
  static inline std::unique_ptr<GFX::StaticBuffer> histogramBuffer;
  static inline const int NUM_BUCKETS = 128;
  static inline std::unique_ptr<GFX::StaticBuffer> floatBufferIn, floatBufferOut;
  static inline std::optional<GFX::TextureView> blueNoiseView;
  static inline std::optional<GFX::TextureSampler> blueNoiseSampler;
  static inline bool tonemapDither = true;
  struct FXAA
  {
    bool enabled{ true };
    float contrastThreshold = 0.0312;
    float relativeThreshold = 0.125;
    float pixelBlendStrength = 0.2;
    float edgeBlendStrength = 1.0;
  }static inline fxaa;
};