#pragma once
#include <vector>

namespace GFX
{
	using MeshID = uint32_t;
	using Index = uint32_t;

	struct Vertex
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 texCoord;
	};

	struct Mesh
	{
		std::vector<Vertex> vertices;
		std::vector<Index> indices;
	};
}