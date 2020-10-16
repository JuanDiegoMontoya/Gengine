#include <GL/glew.h>

#include "Mesh.h"
#include "AssimpUtils.h"

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

unsigned MeshManager::GetFuckingTexture(std::string filename)
{
	if (filename == "")
	{
		unsigned textureId = 0;

		glGenTextures(1, &textureId);
		glBindTexture(GL_TEXTURE_2D, textureId);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 3);

		float culer[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_FLOAT, culer);

		return textureId;
	}
	else
	{
		unsigned textureId = 0;
		int width, height, nrChannels;

		GLubyte* data = stbi_load(filename.c_str(), &width, &height, &nrChannels, 4);

		glGenTextures(1, &textureId);
		glBindTexture(GL_TEXTURE_2D, textureId);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 3);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

		stbi_image_free(data);
		glBindTexture(GL_TEXTURE_2D, 0);

		return textureId;
	}
}

MeshHandle MeshManager::GenHandle_GL(std::vector<unsigned>& indices, std::vector<Vertex>& vertices)
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
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned), &indices[0], GL_STATIC_DRAW);

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

void MeshManager::LoadMesh(const aiScene* scene, aiMesh* mesh, std::vector<unsigned>& indices, std::vector<Vertex>& vertices)
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