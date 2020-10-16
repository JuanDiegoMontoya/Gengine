#pragma once

#include <vector>
#include <string>

#include "MeshUtils.h"

#include <stb_image.h>

class MeshHandle;
class Animation;

struct aiMesh;
struct aiScene;

struct Vertex;
struct VertexAnim;

class MeshManager
{
public:
	static std::vector<MeshHandle> CreateMesh(std::string filename, bool&, bool&);
	
	// Chris: is this temp?
	static unsigned GetFuckingTexture(std::string filename);

private:
	static MeshHandle GenHandle_GL(std::vector<unsigned>&, std::vector<Vertex>&);
	static MeshHandle GenHandle_GL(std::vector<unsigned>&, std::vector<VertexAnim>&);

	static void LoadMesh(const aiScene* scene, aiMesh* mesh, std::vector<unsigned>&, std::vector<Vertex>&);
	static void LoadMesh(const aiScene* scene, aiMesh* mesh, std::vector<unsigned>&, std::vector<VertexAnim>&);
};
