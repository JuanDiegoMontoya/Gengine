#pragma once
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <cinttypes>

using MeshID = uint32_t;

enum class RenderFlags : uint64_t
{
		NoRender = 1,
		Default = 2,
		Unused1 = 4,
		Unused2 = 8,
		Unused3 = 16,
		Unused4 = 32,
		Unused5 = 64,
		Unused6 = 128,
		Unused7 = 256,
		Unused8 = 512,
		Unused9 = 1024,
};

struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoord;
};

struct VertexAnim
{
	glm::uvec4 boneIds = glm::uvec4(0);
	glm::vec4 boneWeights = glm::vec4(0.0f);
};