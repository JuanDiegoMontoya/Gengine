#pragma once

#include <vector>

#include "MeshUtils.h"
#include "Components.h"

class Shader;

class Renderer
{
	public:
		static void Init();

		static void Render(Components::Model& model, Components::Mesh& mesh, Components::Material& mat);

	private:
		static inline Shader* ShaderMcShaderFace = nullptr;
};