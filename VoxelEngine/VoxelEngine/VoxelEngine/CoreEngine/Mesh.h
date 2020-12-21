#pragma once

#include <vector>
#include <string>

#include "MeshUtils.h"

#include <stb_image.h>

struct aiMesh;
struct aiScene;

struct Vertex;
struct VertexAnim;

struct MeshHandle;


class MeshManager
{
public:
	static std::shared_ptr<MeshHandle> CreateMeshBatched(std::string filename, entt::hashed_string name);
	static std::shared_ptr<MeshHandle> GetMeshBatched(entt::hashed_string name);
	static void DestroyBatchedMesh(MeshID handle);
	// TODO: functions to load skeletal meshes

private:
	static void GenBatchedHandle_GL(MeshID handle, const std::vector<GLuint>& indices, const std::vector<Vertex>& vertices);

	static void LoadMesh(const aiScene* scene, aiMesh* mesh, std::vector<GLuint>& indices, std::vector<Vertex>& vertices);
	static void LoadMesh(const aiScene* scene, aiMesh* mesh, std::vector<GLuint>& indices, std::vector<VertexAnim>& vertices);

	static inline std::unordered_map<MeshID, std::pair<uint64_t, uint64_t>> IDMap_;
	static inline std::unordered_map<MeshID, std::weak_ptr<MeshHandle>> handleMap_;
};

struct MeshHandle
{
	MeshHandle(MeshID id) : handle(id) {}
	~MeshHandle() { printf("Destroying mesh, ID %u\n", handle); MeshManager::DestroyBatchedMesh(handle); }

private:
	friend class Renderer;
	MeshID handle;
};