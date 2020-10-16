#pragma once

#include <vector>

#include "MeshUtils.h"
#include "Components.h"

using namespace Components;
class Shader;

class Renderer
{
	public:
		static void Init();

		static void Render(Model& model, Mesh& mesh, Material& mat);

	private:
		static inline Shader* ShaderMcShaderFace = nullptr;
};