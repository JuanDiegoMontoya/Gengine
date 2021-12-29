#pragma once
#include <string_view>
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

	namespace MeshManager
	{
		MeshID CreateMeshBatched(std::string_view filename, hashed_string name);
		MeshID GetMeshBatched(hashed_string name);

		// TODO: function(s) to load skeletal meshes
	};
}