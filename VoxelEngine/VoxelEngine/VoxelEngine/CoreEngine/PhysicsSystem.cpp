#include "PhysicsSystem.h"
#include "Components.h"
#include <entt/src/core/algorithm.hpp>

PhysicsSystem::PhysicsSystem()
{
}

PhysicsSystem::~PhysicsSystem()
{
}

void PhysicsSystem::Update(Scene& scene, float dt)
{
	// gravity + velocity stuff
	{
		auto view = scene.GetRegistry().view<Components::Physics, Components::Transform>();
		for (auto entity : view)
		{
			auto [physics, transform] = view.get<Components::Physics, Components::Transform>(entity);

			physics.velocity.y -= gravity * dt;
			physics.velocity += physics.acceleration;
			transform.SetTranslation(transform.GetTranslation() + physics.velocity);
		}
	}

	// AABB->world collision
	{
		auto view = scene.GetRegistry().view<Components::AABBCollider, Components::Physics, Components::Transform>();
		for (auto entity : view)
		{
			auto [physics, transform] = view.get<Components::Physics, Components::Transform>(entity);


		}
	}

	//ASSERT(scene.GetRegistry().sortable<Components::Transform>());
	//scene.GetRegistry().sort<Components::Transform>(
	//	[&scene](const entt::entity lhs, const entt::entity rhs)
	//	{
	//		return Entity(lhs, &scene).GetHierarchyHeight() > Entity(rhs, &scene).GetHierarchyHeight();
	//	}, entt::insertion_sort());

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
				auto model = glm::mat4(1);
				model *= glm::translate(glm::mat4(1), ltransform.GetTranslation());
				model *= ltransform.GetRotation();
				model *= glm::scale(glm::mat4(1), ltransform.GetScale());
				ltransform.SetModel(model);
			}

			const auto& parentTransform = scene.GetRegistry().get<Components::Transform>(parent.entity);
			if (parentTransform.IsDirty() || localDirty)
			{
				worldTransform.SetTranslation(parentTransform.GetTranslation() + ltransform.GetTranslation() * parentTransform.GetScale());

				worldTransform.SetScale(ltransform.GetScale() * parentTransform.GetScale());

				worldTransform.SetTranslation(worldTransform.GetTranslation() - parentTransform.GetTranslation());
				worldTransform.SetTranslation(glm::mat3(parentTransform.GetRotation()) * worldTransform.GetTranslation());
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
				auto model = glm::mat4(1);
				model *= glm::translate(glm::mat4(1), transform.GetTranslation());
				model *= transform.GetRotation();
				model *= glm::scale(glm::mat4(1), transform.GetScale());
				transform.SetModel(model);
			}
		}
	}
}
