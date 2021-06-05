#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <entt.hpp>

using MeshID = entt::id_type;

enum class RenderFlags : uint64_t
{
		NoRender = 1,
		Default = 2,
		Unused1 = 4,
		Unused2 = 8,
		Unused3 = 16,
		Unused4 = 32,
		Unused5 = 64,
		Unused6 = 128,
		Unused7 = 256,
		Unused8 = 512,
		Unused9 = 1024,
};

struct Vertex
{
	Vertex(glm::vec3 pVertex = glm::vec3(), glm::vec3 pNormal = glm::vec3(), glm::vec2 pTexCoords = glm::vec2()) :
		position(pVertex), normal(pNormal), texCoord(pTexCoords)
	{ }

	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoord;
};

struct VertexAnim
{
	VertexAnim() {}
	VertexAnim(glm::vec3 pVertex, glm::vec3 pNormal = glm::vec3(), glm::vec2 pTexCoords = glm::vec2()) {}

	glm::uvec4 boneIds = glm::uvec4(0);
	glm::vec4 boneWeights = glm::vec4(0.0f);
};