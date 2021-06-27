#include "PCH.h"
#include "PhysicsSystem.h"
#include <entt/src/core/algorithm.hpp>
#include <CoreEngine/Physics.h>

#include "Entity.h"
#include "Components/Physics.h"
#include "Components/Transform.h"

static void OnDynamicPhysicsDelete(entt::basic_registry<entt::entity>& registry, entt::entity entity)
{
  printf("Deleted dynamic physics on entity %d\n", entity);

  auto* physics = registry.get<Component::DynamicPhysics>(entity).internalActor;
  Physics::PhysicsManager::RemoveActorEntity(reinterpret_cast<physx::PxRigidActor*>(physics));
}

static void OnStaticPhysicsDelete(entt::basic_registry<entt::entity>& registry, entt::entity entity)
{
  printf("Deleted static physics on entity %d\n", entity);

  auto* physics = registry.get<Component::StaticPhysics>(entity).internalActor;
  Physics::PhysicsManager::RemoveActorEntity(reinterpret_cast<physx::PxRigidActor*>(physics));
}

static void OnCharacterControllerDelete(entt::basic_registry<entt::entity>& registry, entt::entity entity)
{
  printf("Deleted character controller on entity %d\n", entity);

  auto* controller = registry.get<Component::CharacterController>(entity).internalController;
  Physics::PhysicsManager::RemoveCharacterControllerEntity(controller);
}

PhysicsSystem::PhysicsSystem()
{
	Physics::PhysicsManager::Init();
}

PhysicsSystem::~PhysicsSystem()
{
	Physics::PhysicsManager::Shutdown();
}

void PhysicsSystem::InitScene(Scene& scene)
{
  scene.GetRegistry().on_destroy<Component::DynamicPhysics>()
    .connect<&OnDynamicPhysicsDelete>();
  scene.GetRegistry().on_destroy<Component::StaticPhysics>()
    .connect<&OnStaticPhysicsDelete>();
  scene.GetRegistry().on_destroy<Component::CharacterController>()
    .connect<&OnCharacterControllerDelete>();
}

void PhysicsSystem::Update(Scene& scene, float dt)
{
	// update local transforms
	{
		using namespace Component;
		// create a PARTIALLY OWNING group, OWNING TRANSFORM
		auto group = scene.GetRegistry().group<Transform>(entt::get<LocalTransform, Parent>);
		group.sort(
			[&scene](const entt::entity lhs, const entt::entity rhs)
		{
			return Entity(lhs, &scene).GetHierarchyHeight() > Entity(rhs, &scene).GetHierarchyHeight();
		}, entt::insertion_sort()); // insertion sort optimal for nearly-sorted containers

		for (auto entity : group)
		{
			auto [worldTransform, localTransform, parent] = group.get<Transform, LocalTransform, Parent>(entity);
			auto& ltransform = localTransform.transform;
			bool localDirty = ltransform.IsDirty();
			if (ltransform.IsDirty())
			{
				ltransform.SetModel();
			}

			const auto& parentTransform = scene.GetRegistry().get<Component::Transform>(parent.entity);
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
		auto view = scene.GetRegistry().view<Component::Transform>();
		for (auto entity : view)
		{
			auto& transform = view.get<Component::Transform>(entity);
			if (transform.IsDirty())
			{
				transform.SetModel();
			}
		}
	}

	Physics::PhysicsManager::Simulate(dt);
}
