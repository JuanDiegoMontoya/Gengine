#pragma once

#include <vector>

#include "MeshUtils.h"
#include "Components.h"

using namespace Components;
class Shader;

class Renderer
{
	public:
		Renderer();
		~Renderer();

		void Init();

		void Render(Model& model, Mesh& mesh, Material& mat);

	private:
		Shader* ShaderMcShaderFace = nullptr;
};