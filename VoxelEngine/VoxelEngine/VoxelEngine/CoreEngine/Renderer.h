#pragma once

#include <vector>

#include "MeshUtils.h"
#include "Components.h"

class Shader;

class Renderer
{
public:
	static void Init();
	static void CompileShaders();

	static void Render(Components::Transform& model, Components::Mesh& mesh, Components::Material& mat);

	// generic drawing functions (TODO: move)
	static void DrawAxisIndicator();
	static void DrawQuad();
	static void DrawCube();

private:
	static inline Shader* ShaderMcShaderFace = nullptr;
};