#pragma once

struct AABB;
struct AABB16;

struct AABB
{
	AABB() = default;
	AABB(const glm::vec3& min_, const glm::vec3& max_) : min(min_), max(max_) {}
	AABB(const AABB16& b);
	glm::vec3 min, max;
};

// vecs are 16-byte aligned for GPU usage
// the .w component is unused
struct AABB16
{
	AABB16() = default;
	AABB16(const AABB& b);
	glm::vec4 min, max;
};

inline AABB::AABB(const AABB16& b) : min(b.min), max(b.max) {}
inline AABB16::AABB16(const AABB& b) : min(glm::vec4(b.min, 0)), max(glm::vec4(b.max, 0)) {}