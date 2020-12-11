#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct MeshHandle
{
	unsigned VAO = 0;

	int indexCount = 0;
};

struct BatchedMeshHandle
{
	auto operator<=>(const BatchedMeshHandle&) const = default;
	unsigned handle;
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