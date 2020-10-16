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

	static unsigned GetFuckingTexture(std::string filename)
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

private:
	static MeshHandle GenHandle_GL(std::vector<unsigned>&, std::vector<Vertex>&);
	static MeshHandle GenHandle_GL(std::vector<unsigned>&, std::vector<VertexAnim>&);

	static void LoadMesh(const aiScene* scene, aiMesh* mesh, std::vector<unsigned>&, std::vector<Vertex>&);
	static void LoadMesh(const aiScene* scene, aiMesh* mesh, std::vector<unsigned>&, std::vector<VertexAnim>&);
};
