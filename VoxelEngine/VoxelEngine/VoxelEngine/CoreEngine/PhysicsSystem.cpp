#include "EnginePCH.h"
#include "PhysicsSystem.h"
#include "Components.h"
#include <entt/src/core/algorithm.hpp>

#include <CoreEngine/Physics.h>

PhysicsSystem::PhysicsSystem()
{
	Physics::PhysicsManager::Init();
}

PhysicsSystem::~PhysicsSystem()
{
	Physics::PhysicsManager::Shutdown();
}

void PhysicsSystem::Update(Scene& scene, float dt)
{
	// update local transforms
	{
		using namespace Components;
		// create a PARTIALLY OWNING group, OWNING TRANSFORM
		auto group = scene.GetRegistry().group<Transform>(entt::get<LocalTransform, Parent>);
		group.sort(
			[&scene](const entt::entity lhs, const entt::entity rhs)
		{
			return Entity(lhs, &scene).GetHierarchyHeight() > Entity(rhs, &scene).GetHierarchyHeight();
		}, entt::insertion_sort());

		for (auto entity : group)
		{
			auto [worldTransform, localTransform, parent] = group.get<Transform, LocalTransform, Parent>(entity);
			auto& ltransform = localTransform.transform;
			bool localDirty = ltransform.IsDirty();
			if (ltransform.IsDirty())
			{
				ltransform.SetModel();
			}

			const auto& parentTransform = scene.GetRegistry().get<Components::Transform>(parent.entity);
			if (parentTransform.IsDirty() || localDirty)
			{
				worldTransform.SetTranslation(parentTransform.GetTranslation() + ltransform.GetTranslation() * parentTransform.GetScale());

				worldTransform.SetScale(ltransform.GetScale() * parentTransform.GetScale());

				worldTransform.SetTranslation(worldTransform.GetTranslation() - parentTransform.GetTranslation());
				worldTransform.SetTranslation(glm::mat3(glm::mat4_cast(parentTransform.GetRotation())) * worldTransform.GetTranslation());
				worldTransform.SetTranslation(worldTransform.GetTranslation() + parentTransform.GetTranslation());

				worldTransform.SetRotation(ltransform.GetRotation() * parentTransform.GetRotation());
			}
		}
	}

	// update model matrices after potential changes
	{
		auto view = scene.GetRegistry().view<Components::Transform>();
		for (auto entity : view)
		{
			auto& transform = view.get<Components::Transform>(entity);
			if (transform.IsDirty())
			{
				transform.SetModel();
			}
		}
	}

	Physics::PhysicsManager::Simulate(dt);
}
