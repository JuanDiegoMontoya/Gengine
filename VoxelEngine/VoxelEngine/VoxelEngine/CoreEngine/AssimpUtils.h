#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/vector3.h>
#include <assimp/Importer.hpp>

inline glm::mat4 AssimpToGlm(aiMatrix4x4 mat)
{
	glm::mat4 m;
	for (int y = 0; y < 4; y++)
	{
		for (int x = 0; x < 4; x++)
		{
			m[x][y] = mat[y][x];
		}
	}
	return m;
}

inline glm::vec3 AssimpToGlm(aiVector3D vec)
{
	return glm::vec3(vec.x, vec.y, vec.z);
}

inline glm::vec4 AssimpToGlm(aiColor4D vec)
{
	return glm::vec4(vec.r, vec.g, vec.b, vec.a);
}

inline glm::vec3 AssimpToGlm(aiColor3D vec)
{
	return glm::vec3(vec.r, vec.g, vec.b);
}

inline glm::quat AssimpToGlm(aiQuaternion quat)
{
	glm::quat q;
	q.x = quat.x;
	q.y = quat.y;
	q.z = quat.z;
	q.w = quat.w;

	return q;
}
