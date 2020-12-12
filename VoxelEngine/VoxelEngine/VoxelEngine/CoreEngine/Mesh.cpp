#include "EnginePCH.h"
#include <GL/glew.h>

#include "Mesh.h"
#include "AssimpUtils.h"
#include <CoreEngine/Renderer.h>

#include <algorithm>
#include <iostream>

static Assimp::Importer importer;

std::vector<MeshHandle> MeshManager::CreateMesh(std::string filename, bool& hasSkeleton, bool& hasAnimations)
{
	const aiScene* scene = importer.ReadFile(filename.c_str(), aiProcessPreset_TargetRealtime_Fast);
	
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::string rre = importer.GetErrorString();
		std::cout << "ERROR::ASSIMP::" << rre << std::endl;
	}

	hasAnimations = scene->HasAnimations();
	hasSkeleton = scene->mMeshes[0]->HasBones();

	std::vector<MeshHandle> meshHandles;

	for (unsigned m = 0; m < scene->mNumMeshes; m++)
	{
		std::vector<Vertex> vertices;
		std::vector<unsigned> indices;

		LoadMesh(scene, scene->mMeshes[m], indices, vertices);
		meshHandles.push_back(GenHandle_GL(indices, vertices));
	}

	return meshHandles;
}

std::vector<BatchedMeshHandle> MeshManager::CreateMeshBatched(std::string filename, bool& hasSkeleton, bool& hasAnimations)
{
	const aiScene* scene = importer.ReadFile(filename.c_str(), aiProcessPreset_TargetRealtime_Fast);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::string rre = importer.GetErrorString();
		std::cout << "ERROR::ASSIMP::" << rre << std::endl;
	}

	hasAnimations = scene->HasAnimations();
	hasSkeleton = scene->mMeshes[0]->HasBones();

	std::vector<BatchedMeshHandle> meshHandles;

	for (unsigned m = 0; m < scene->mNumMeshes; m++)
	{
		std::vector<Vertex> vertices;
		std::vector<unsigned> indices;

		LoadMesh(scene, scene->mMeshes[m], indices, vertices);
		meshHandles.push_back(GenBatchedHandle_GL(indices, vertices));
	}

	return meshHandles;
}

MeshHandle MeshManager::GenHandle_GL(std::vector<GLuint>& indices, std::vector<Vertex>& vertices)
{
	MeshHandle mh;

	GLuint VBO, IBO;

	glGenVertexArrays(1, &mh.VAO);
	glBindVertexArray(mh.VAO);

	glGenBuffers(1, &VBO);
	glGenBuffers(1, &IBO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, position));
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, normal));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, texCoord));

	glBindVertexArray(0);

	mh.indexCount = indices.size();

	return mh;
}

BatchedMeshHandle MeshManager::GenBatchedHandle_GL(const std::vector<GLuint>& indices, const std::vector<Vertex>& vertices)
{
	// never freed
	auto vh = Renderer::vertexBuffer->Allocate(vertices.data(), vertices.size() * sizeof(Vertex));
	auto ih = Renderer::indexBuffer->Allocate(indices.data(), indices.size() * sizeof(GLuint));
	const auto& vinfo = Renderer::vertexBuffer->GetAlloc(vh);
	const auto& iinfo = Renderer::indexBuffer->GetAlloc(ih);
	BatchedMeshHandle handle{ .handle = Renderer::nextHandle++ };
	
	// generate an indirect draw command with most of the info needed to draw this mesh
	DrawElementsIndirectCommand cmd{};
	cmd.baseVertex = vinfo.offset / Renderer::vertexBuffer->align_;
	cmd.instanceCount = 0;
	cmd.count = indices.size();
	cmd.firstIndex = iinfo.offset / Renderer::indexBuffer->align_;
	//cmd.baseInstance = 0; // only knowable after all user draw calls are submitted
	Renderer::meshBufferInfo[handle] = cmd;
	return handle;
}

void MeshManager::LoadMesh(const aiScene* scene, aiMesh* mesh, std::vector<GLuint>& indices, std::vector<Vertex>& vertices)
{
	for (unsigned i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;

		glm::vec3 vector;
		vector.x = mesh->mVertices[i].x;
		vector.y = mesh->mVertices[i].y;
		vector.z = mesh->mVertices[i].z;
		vertex.position = vector;

		vector = glm::vec3();
		if (mesh->HasNormals())
		{
			vector.x = mesh->mNormals[i].x;
			vector.y = mesh->mNormals[i].y;
			vector.z = mesh->mNormals[i].z;
		}
		vertex.normal = vector;

		glm::vec2 vec = glm::vec2();
		if (mesh->HasTextureCoords(0))
		{
			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;
		}
		vertex.texCoord = vec;

		vertices.push_back(vertex);
	}

	for (unsigned i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace& face = mesh->mFaces[i];
		for (unsigned j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	}
}