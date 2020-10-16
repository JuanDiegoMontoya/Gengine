#include "PhysicsSystem.h"
#include "Components.h"

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
		auto group = scene.GetRegistry().group<Components::Transform>(entt::get<Components::Physics>);
		for (auto entity : group)
		{
			auto [transform, physics] = group.get<Components::Transform, Components::Physics>(entity);

			physics.velocity.y -= gravity * dt;
			transform.translation += physics.velocity;
		}
	}

	// AABB->world collision
	{
		auto group = scene.GetRegistry().group<Components::Transform>(entt::get<Components::Physics, Components::AABBCollider>);
		for (auto entity : group)
		{
			auto [transform, physics] = group.get<Components::Transform, Components::Physics>(entity);


		}
	}
}
