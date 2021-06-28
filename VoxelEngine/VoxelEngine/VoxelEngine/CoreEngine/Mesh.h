#pragma once

#include <vector>
#include <string>

#include "MeshUtils.h"

#include <stb_image.h>
#include "utilities.h"

struct aiMesh;
struct aiScene;

struct Vertex;
struct VertexAnim;

class MeshManager
{
public:
	static MeshID CreateMeshBatched(const std::string& filename, hashed_string name);
	static MeshID GetMeshBatched(hashed_string name);

	// TODO: functions to load skeletal meshes

private:
	friend class GraphicsSystem;

	static void GenBatchedHandle_GL(hashed_string handle, const std::vector<GLuint>& indices, const std::vector<Vertex>& vertices);

	static void LoadMesh(const aiScene* scene, aiMesh* mesh, std::vector<GLuint>& indices, std::vector<Vertex>& vertices);
	static void LoadMesh(const aiScene* scene, aiMesh* mesh, std::vector<GLuint>& indices, std::vector<VertexAnim>& vertices);

	static inline std::unordered_map<hashed_string, std::pair<uint64_t, uint64_t>> IDMap_;
	static inline std::unordered_map<hashed_string, MeshID> handleMap_;
};