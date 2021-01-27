#pragma once
#include <vector>

#include "MeshUtils.h"
#include "Components.h"
#include <CoreEngine/DynamicBuffer.h>
#include <shared_mutex>
#include <CoreEngine/Fence.h>

class Shader;
namespace GFX
{
  class Fence;
}

class Renderer
{
public:
  static void Init();
  static void CompileShaders();

  static void BeginBatch(size_t size);
  static void Submit(const Components::Transform& model, const Components::BatchedMesh& mesh, const Components::Material& mat);
  static void RenderBatch();
  static void BeginRenderParticleEmitter();
  static void RenderParticleEmitter(const Components::ParticleEmitter& emitter, const Components::Transform& model);

  // generic drawing functions (TODO: move)
  static void DrawAxisIndicator();
  static void DrawQuad();
  static void DrawCube();

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
  static inline std::unique_ptr<GFX::VAO> batchVAO;

  // maps handles to VERTEX and INDEX information in the respective dynamic buffers
  // used to retrieve important offset and size info for meshes
  using DBaT = GFX::DynamicBuffer<>::allocationData<>;
  static inline std::map<entt::id_type, DrawElementsIndirectCommand> meshBufferInfo;

  struct BatchDrawCommand
  {
    MeshID mesh;
    MaterialID material;
    glm::mat4 modelUniform;
  };
  static inline std::vector<BatchDrawCommand> userCommands;
  static inline std::atomic_uint32_t cmdIndex{ 0 };

  static inline std::unique_ptr<GFX::VAO> emptyVao;

  // HDR inverse-z framebuffer stuff
  static inline unsigned fbo;
  static inline int windowWidth = 1920, windowHeight = 1017;
  static inline int fboWidth = 1920, fboHeight = 1017;
  //static inline int windowWidth = 10, windowHeight = 10;
  //static inline int fboWidth = 10, fboHeight = 10;
  static inline GLuint color;
  static inline GLuint depth;
  static inline float exposure = 1.0f;
  static inline float minExposure = 0.1f;
  static inline float maxExposure = 10.0f;
  static inline float targetLuminance = .22f;
  static inline float adjustmentSpeed = 0.5f;
  static inline bool tonemapping = true;
  static inline bool gammaCorrection = true;
  static inline std::unique_ptr<GFX::StaticBuffer> exposureBuffer;
  static inline std::unique_ptr<GFX::StaticBuffer> histogramBuffer;
  static inline const int NUM_BUCKETS = 128;
  static inline std::unique_ptr<GFX::StaticBuffer> floatBufferIn, floatBufferOut;
};

void compute_test();
void histogram_test();
