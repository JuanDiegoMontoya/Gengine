#include "../PCH.h"
#include <glad/glad.h>

#include "Mesh.h"
#include "AssimpUtils.h"
#include <engine/gfx/Renderer.h>

#include <algorithm>
#include <iostream>

static Assimp::Importer importer;

MeshID MeshManager::CreateMeshBatched(const std::string& filename, hashed_string name)
{
	const aiScene* scene = importer.ReadFile(filename.c_str(), aiProcessPreset_TargetRealtime_Fast);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::string rre = importer.GetErrorString();
		std::cout << "ERROR::ASSIMP::" << rre << std::endl;
	}

	if (scene->HasAnimations())
	{
		printf("Animation loading not supported by this function.\n");
	}
	if (scene->mMeshes[0]->HasBones())
	{
		printf("Bone loading not supported by this function.\n");
	}

	std::vector<MeshID> meshHandles;
	
	if (scene->mNumMeshes == 0)
	{
		printf("File does not contain a mesh.\n");
	}
	if (scene->mNumMeshes > 1)
	{
		printf("Multiple mesh loading not supported.\n");
	}

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	LoadMesh(scene, scene->mMeshes[0], indices, vertices);

	GenBatchedHandle_GL(name, indices, vertices);
  handleMap_[name] = name;
	return name;
}

MeshID MeshManager::GetMeshBatched(hashed_string name)
{
	return handleMap_[name];
}

void MeshManager::GenBatchedHandle_GL(hashed_string handle, const std::vector<uint32_t>& indices, const std::vector<Vertex>& vertices)
{
	// never freed
	auto vh = GFX::Renderer::vertexBuffer->Allocate(vertices.data(), vertices.size() * sizeof(Vertex));
	auto ih = GFX::Renderer::indexBuffer->Allocate(indices.data(), indices.size() * sizeof(uint32_t));
	const auto& vinfo = GFX::Renderer::vertexBuffer->GetAlloc(vh);
	const auto& iinfo = GFX::Renderer::indexBuffer->GetAlloc(ih);
	IDMap_[handle] = { vh, ih };
	
	// generate an indirect draw command with most of the info needed to draw this mesh
	DrawElementsIndirectCommand cmd{};
	cmd.baseVertex = vinfo.offset / GFX::Renderer::vertexBuffer->align_;
	cmd.instanceCount = 0;
	cmd.count = static_cast<uint32_t>(indices.size());
	cmd.firstIndex = iinfo.offset / GFX::Renderer::indexBuffer->align_;
	//cmd.baseInstance = ?; // only knowable after all user draw calls are submitted
  GFX::Renderer::meshBufferInfo[handle] = cmd;
}

//void MeshManager::DestroyBatchedMesh(MeshID handle)
//{
//  auto [vh, ih] = std::find_if(IDMap_.begin(), IDMap_.end(), [&](const auto& elem) { return elem.first.value() == handle; })->second;
//  //auto [vh, ih] = IDMap_[handle];
//	Renderer::vertexBuffer->Free(vh);
//	Renderer::indexBuffer->Free(ih);
//	Renderer::meshBufferInfo.erase(handle);
//  IDMap_.erase(std::find_if(IDMap_.begin(), IDMap_.end(), [&](const auto& elem) { return elem.first.value() == handle; }));
//  handleMap_.erase(std::find_if(handleMap_.begin(), handleMap_.end(), [&](const auto& elem) { return elem.first.value() == handle; }));
//	//IDMap_.erase(handle);
//	//handleMap_.erase(handle);
//}

void MeshManager::LoadMesh([[maybe_unused]] const aiScene* scene, aiMesh* mesh, std::vector<uint32_t>& indices, std::vector<Vertex>& vertices)
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