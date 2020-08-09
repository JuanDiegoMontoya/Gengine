#include "Scene.h"
#include "Entity.h"
#include "Components.h"

Scene::Scene()
{
}

Scene::~Scene()
{
}

Entity Scene::CreateEntity(std::string_view name)
{
	Entity entity = { registry_.create(), this };
	entity.AddComponent<TransformComponent>();
	auto& nome = entity.AddComponent<NameComponent>();
	nome.name = name;
}

void Scene::Update(double dt)
{
	// Render 2D
	glm::mat4* cameraTransform = nullptr;
	auto group = registry_.view<TransformComponent, CameraComponent>();
	for (auto entity : group)
	{
		auto [transform, camera] = group.get<TransformComponent, CameraComponent>(entity);
		if (mainCamera == &camera.camera)
		{
			cameraTransform = &transform.transform;
			break;
		}
		//if (camera.Primary)
		//{
		//	mainCamera = &camera.Camera;
		//	cameraTransform = &transform.Transform;
		//	break;
		//}
	}

	//if (mainCamera)
	//{
	//	Renderer2D::BeginScene(mainCamera->GetProjection(), *cameraTransform);

	//	auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
	//	for (auto entity : group)
	//	{
	//		auto& [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);

	//		Renderer2D::DrawQuad(transform, sprite.Color);
	//	}

	//	Renderer2D::EndScene();
	//}
	if (mainCamera)
	{
		// transform + mesh + render + ??? components to renderer to be rendered
		// full-owning group
		auto group = registry_.group<TransformComponent, MeshComponent, RenderableComponent>();
		for (auto entity : group)
		{
			auto [transform, mesh, render] = group.get<TransformComponent, MeshComponent, RenderableComponent>(entity);
		}
	}
}
