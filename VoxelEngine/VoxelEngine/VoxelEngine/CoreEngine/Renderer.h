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

  static void BeginBatch(uint32_t size);
  static void Submit(const Components::Transform& model, const Components::BatchedMesh& mesh, const Components::Material& mat);
  static void RenderBatch();
  static void BeginRenderParticleEmitter();
  static void RenderParticleEmitter(const Components::ParticleEmitter& emitter, const Components::Transform& model);

  // generic drawing functions (TODO: move)
  static void DrawAxisIndicator();
  static void DrawQuad();
  static void DrawCube();

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

  static inline std::unique_ptr<GFX::VAO> particleVao;
};