#pragma once

#include <vector>

#include "MeshUtils.h"

class Shader;

struct Material;
struct Mesh;
struct Model;

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