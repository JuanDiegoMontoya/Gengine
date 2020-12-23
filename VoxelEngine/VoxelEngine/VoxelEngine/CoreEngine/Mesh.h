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

struct MeshHandle;


class MeshManager
{
public:
	static std::shared_ptr<MeshHandle> CreateMeshBatched(std::string filename, entt::hashed_string name);
	static std::shared_ptr<MeshHandle> GetMeshBatched(entt::hashed_string name);
	static void DestroyBatchedMesh(MeshID handle);
	// TODO: functions to load skeletal meshes

private:
	friend class GraphicsSystem;

	static void GenBatchedHandle_GL(entt::hashed_string handle, const std::vector<GLuint>& indices, const std::vector<Vertex>& vertices);

	static void LoadMesh(const aiScene* scene, aiMesh* mesh, std::vector<GLuint>& indices, std::vector<Vertex>& vertices);
	static void LoadMesh(const aiScene* scene, aiMesh* mesh, std::vector<GLuint>& indices, std::vector<VertexAnim>& vertices);

	static inline std::unordered_map<entt::hashed_string, std::pair<uint64_t, uint64_t>> IDMap_;
	static inline std::unordered_map<entt::hashed_string, std::weak_ptr<MeshHandle>> handleMap_;
};

struct MeshHandle
{
	MeshHandle(MeshID id) : handle(id) {}
	~MeshHandle() { printf("Destroying mesh, ID %u\n", handle); MeshManager::DestroyBatchedMesh(handle); }

private:
	friend class Renderer;
	MeshID handle;
};