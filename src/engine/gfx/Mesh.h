#pragma once

#include <vector>
#include <string>

#include "MeshUtils.h"

#include "../utilities.h"

struct aiMesh;
struct aiScene;

struct Vertex;
struct VertexAnim;

namespace MeshManager
{
	MeshID CreateMeshBatched(const std::string& filename, hashed_string name);
	MeshID GetMeshBatched(hashed_string name);

	// TODO: functions to load skeletal meshes

	void GenBatchedHandle_GL(hashed_string handle, const std::vector<uint32_t>& indices, const std::vector<Vertex>& vertices);

	void LoadMesh(const aiScene* scene, aiMesh* mesh, std::vector<uint32_t>& indices, std::vector<Vertex>& vertices);
	void LoadMesh(const aiScene* scene, aiMesh* mesh, std::vector<uint32_t>& indices, std::vector<VertexAnim>& vertices);
};