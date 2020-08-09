#pragma once

#include <glm/glm.hpp>
#include <camera.h>
#include <string>

struct NameComponent
{
	std::string name;

	NameComponent() = default;
	NameComponent(const NameComponent&) = default;
	NameComponent(const std::string& name)
		: name(name) {}
};

struct TransformComponent
{
	glm::mat4 transform{ 1.0f };

	TransformComponent() = default;
	TransformComponent(const TransformComponent&) = default;
	TransformComponent(const glm::mat4& transform)
		: transform(transform) {}

	operator glm::mat4& () { return transform; }
	operator const glm::mat4& () const { return transform; }
};

struct SpriteRendererComponent
{
	glm::vec4 color{ 1.0f, 1.0f, 1.0f, 1.0f };

	SpriteRendererComponent() = default;
	SpriteRendererComponent(const SpriteRendererComponent&) = default;
	SpriteRendererComponent(const glm::vec4& color)
		: color(color) {}
};

struct CameraComponent
{
	Camera camera;
	//bool Primary = true; // TODO: move to Scene class

	CameraComponent() = default;
	CameraComponent(const CameraComponent&) = default;
	//CameraComponent(const glm::mat4& projection)
	//	: Camera(projection) {}
};

struct MeshComponent
{
	// add mesh data here
	int dummy = 00;
};

struct RenderableComponent
{
	bool renderable = true;
};