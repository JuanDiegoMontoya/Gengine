#pragma once

#include <vector>

#include "MeshUtils.h"
#include "Components.h"
#include <CoreEngine/DynamicBuffer.h>

class Shader;


class Renderer
{
public:
	static void Init();
	static void CompileShaders();

	static void Render(Components::Transform& model, Components::Mesh& mesh, Components::Material& mat);
	static void Submit(Components::Transform& model, Components::Mesh& mesh, Components::Material& mat);
	static void RenderBatch();

	// generic drawing functions (TODO: move)
	static void DrawAxisIndicator();
	static void DrawQuad();
	static void DrawCube();

private:
	// batched+instanced rendering stuff (ONE MATERIAL SUPPORTED ATM)
	friend class MeshManager;
	static inline std::vector<DrawElementsIndirectCommand> indirectCommands;
	static inline std::unique_ptr<DynamicBuffer<>> vertexBuffer;
	static inline std::unique_ptr<DynamicBuffer<>> indexBuffer;
	static inline std::unique_ptr<StaticBuffer> uniformsUBO; // uniform per object instance

	static inline std::unique_ptr<VAO> batchVAO;
	static inline std::unique_ptr<StaticBuffer> batchDIB;

	// changes when a mesh is added, tells Renderer 
	static inline bool dirtyBuffers;
};