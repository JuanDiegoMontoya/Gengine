#pragma once

#include <vector>

#include "MeshUtils.h"
#include "Components.h"
#include <CoreEngine/DynamicBuffer.h>
#include <shared_mutex>

class Shader;

namespace std
{
	template<>
	struct hash<BatchedMeshHandle>
	{
		std::size_t operator()(const BatchedMeshHandle& k) const
		{
			return std::hash<unsigned>()(k.handle);
		}
	};
}

class Renderer
{
public:
	static void Init();
	static void CompileShaders();

	static void Render(Components::Transform& model, Components::Mesh& mesh, Components::Material& mat);
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
	static inline std::unique_ptr<GPU::DynamicBuffer<>> vertexBuffer;
	static inline std::unique_ptr<GPU::DynamicBuffer<>> indexBuffer;

	// per-vertex layout
	static inline std::unique_ptr<GPU::VAO> batchVAO;

	// maps handles to VERTEX and INDEX information in the respective dynamic buffers
	// used to retrieve important offset and size info for meshes
	using DBaT = GPU::DynamicBuffer<>::allocationData<>;
	static inline std::unordered_map<BatchedMeshHandle, DrawElementsIndirectCommand> meshBufferInfo;
	static inline unsigned nextHandle = 1;

	struct BatchDrawCommand
	{
		BatchedMeshHandle mesh;
		MaterialHandle material;
		glm::mat4 modelUniform;
	};
	static inline std::vector<BatchDrawCommand> userCommands;

	static inline std::atomic_uint32_t cmdIndex{ 0 };
	//static inline std::vector<DrawElementsIndirectCommand> indirectCommands;
};