#pragma once

#include <GL/glew.h>

struct MeshHandle
{
	GLuint VAO = 0;
	GLuint Texture = 0;

	int indexCount = 0;
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