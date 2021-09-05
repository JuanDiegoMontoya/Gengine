#include "../../PCH.h"
#include "PhysicsSystem.h"
#include <entt/core/algorithm.hpp>
#include <engine/Physics.h>
#include <glm/gtx/compatibility.hpp>
#include <execution>
#include <engine/core/StatMacros.h>

#include "../Entity.h"
#include "../component/Physics.h"
#include "../component/Transform.h"

DECLARE_FLOAT_STAT(TransformUpdate, CPU)
DECLARE_FLOAT_STAT(PhysicsSimulate, CPU)

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

void PhysicsSystem::Update(Scene& scene, Timestep timestep)
{
  {
    MEASURE_CPU_TIMER_STAT(TransformUpdate);

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
          //model.matrix = worldTransform.GetModel();
          ltransform.SetModel();
        }

        const auto& parentTransform = parent.entity.GetComponent<Transform>();
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
      auto view = scene.GetRegistry().view<Component::Transform, Component::Model>(entt::exclude<Component::InterpolatedPhysics>);
      std::for_each(std::execution::par_unseq, view.begin(), view.end(), [&view](auto entity)
        //for (auto entity : view)
        {
          auto [transform, model] = view.get<Component::Transform, Component::Model>(entity);
          if (transform.IsDirty())
          {
            model.matrix = transform.GetModel();
            transform.SetModel();
          }
        });
    }

    {
      using namespace Component;
      auto view = scene.GetRegistry().view<Model, Transform, InterpolatedPhysics>();
      std::for_each(std::execution::par_unseq, view.begin(), view.end(), [timestep, &view](auto entity)
        //for (auto entity : view)
        {
          auto [model, transform, interp] = view.get<Model, Transform, InterpolatedPhysics>(entity);
          transform.SetModel();
          if (interp.timeSinceUpdate < 0)
          {
            if (interp.timeSinceUpdate == -1)
              model.matrix = transform.GetModel();
            //continue;
            return;
          }
          float lerpAmt = glm::clamp(interp.timeSinceUpdate / (float)::Physics::PhysicsManager::GetStep(), 0.0f, 1.0f);
          Transform trinterp;
          trinterp.SetTranslation(glm::lerp(interp.prevPos, transform.GetTranslation(), lerpAmt));
          trinterp.SetRotation(glm::slerp(interp.prevRot, transform.GetRotation(), lerpAmt));
          trinterp.SetScale(transform.GetScale());
          model.matrix = trinterp.GetModel();
          interp.timeSinceUpdate += timestep.dt_effective;
        });
    }
  }

  {
    MEASURE_CPU_TIMER_STAT(PhysicsSimulate);
    Physics::PhysicsManager::Simulate(timestep);
  }
}
