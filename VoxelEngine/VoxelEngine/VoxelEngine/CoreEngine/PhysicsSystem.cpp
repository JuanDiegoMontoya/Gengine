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
		auto view = scene.GetRegistry().view<Components::Physics, Components::Transform>();
		for (auto entity : view)
		{
			auto [physics, transform] = view.get<Components::Physics, Components::Transform>(entity);

			physics.velocity.y -= gravity * dt;
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
