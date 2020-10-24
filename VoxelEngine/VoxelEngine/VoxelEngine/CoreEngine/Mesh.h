#pragma once

#include <vector>
#include <string>

#include "MeshUtils.h"

#include <stb_image.h>

struct MeshHandle;
struct BatchedMeshHandle;
class Animation;

struct aiMesh;
struct aiScene;

struct Vertex;
struct VertexAnim;

class MeshManager
{
public:
	static std::vector<MeshHandle> CreateMesh(std::string filename, bool& hasSkeleton, bool& hasAnimations);
	static std::vector<BatchedMeshHandle> CreateMeshBatched(std::string filename, bool& hasSkeleton, bool& hasAnimations);

private:
	static MeshHandle GenHandle_GL(std::vector<GLuint>&, std::vector<Vertex>&);
	static BatchedMeshHandle GenBatchedHandle_GL(const std::vector<GLuint>& indices, const std::vector<Vertex>& vertices);
	static MeshHandle GenHandle_GL(std::vector<GLuint>&, std::vector<VertexAnim>&);

	static void LoadMesh(const aiScene* scene, aiMesh* mesh, std::vector<unsigned>&, std::vector<Vertex>&);
	static void LoadMesh(const aiScene* scene, aiMesh* mesh, std::vector<unsigned>&, std::vector<VertexAnim>&);
};
