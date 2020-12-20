#pragma once

#include <vector>

#include "MeshUtils.h"
#include "Components.h"
#include <CoreEngine/DynamicBuffer.h>
#include <shared_mutex>

class Shader;

class Renderer
{
public:
	static void Init();
	static void CompileShaders();

	static void BeginBatch(uint32_t size);
	static void Submit(Components::Transform& model, Components::BatchedMesh& mesh, Components::Material& mat);
	static void RenderBatch();

	// generic drawing functions (TODO: move)
	static void DrawAxisIndicator();
	static void DrawQuad();
	static void DrawCube();

private:
	// std140
	struct UniformData
	{
		glm::mat4 model;
	};
	static void RenderBatchHelper(MaterialHandle material, const std::vector<UniformData>& uniformBuffer);

	// batched+instanced rendering stuff (ONE MATERIAL SUPPORTED ATM)
	friend class MeshManager;
	static inline std::unique_ptr<GFX::DynamicBuffer<>> vertexBuffer;
	static inline std::unique_ptr<GFX::DynamicBuffer<>> indexBuffer;

	// per-vertex layout
	static inline std::unique_ptr<GFX::VAO> batchVAO;

	// maps handles to VERTEX and INDEX information in the respective dynamic buffers
	// used to retrieve important offset and size info for meshes
	using DBaT = GFX::DynamicBuffer<>::allocationData<>;
	static inline std::unordered_map<uint32_t, DrawElementsIndirectCommand> meshBufferInfo;

	struct BatchDrawCommand
	{
		MeshID mesh;
		MaterialHandle material;
		glm::mat4 modelUniform;
	};
	static inline std::vector<BatchDrawCommand> userCommands;

	static inline std::atomic_uint32_t cmdIndex{ 0 };
};