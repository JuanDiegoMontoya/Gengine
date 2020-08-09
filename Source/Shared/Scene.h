#pragma once

#include <entt.hpp>

class Entity;
class Camera;

class Scene
{
public:

	Scene();
	~Scene();

	Entity CreateEntity(std::string_view name);
	void Update(double dt);
	void SetMainCamera(Camera* camera) { mainCamera = camera; }

	entt::registry& GetRegistry() { return registry_; }

private:
	friend class Entity;
	entt::registry registry_;
	Camera* mainCamera = nullptr;
};