#pragma once
#include "../Physics.h"

namespace Component
{
  struct DynamicPhysics
  {
    DynamicPhysics(Entity ent, ::Physics::MaterialType mat, const ::Physics::BoxCollider& c, ::Physics::DynamicActorFlags flags = ::Physics::DynamicActorFlags{})
    {
      internalActor = ::Physics::PhysicsManager::AddDynamicActorEntity(ent, mat, c, flags);
    }
    DynamicPhysics(Entity ent, ::Physics::MaterialType mat, const ::Physics::CapsuleCollider& c, ::Physics::DynamicActorFlags flags = ::Physics::DynamicActorFlags{})
    {
      internalActor = ::Physics::PhysicsManager::AddDynamicActorEntity(ent, mat, c, flags);
    }

    ::Physics::DynamicActorInterface Interface()
    {
      return internalActor;
    }

    physx::PxRigidDynamic* internalActor;
  };

  struct StaticPhysics
  {
    StaticPhysics(Entity ent, ::Physics::MaterialType mat, const ::Physics::BoxCollider& c)
    {
      internalActor = ::Physics::PhysicsManager::AddStaticActorEntity(ent, mat, c);
    }
    StaticPhysics(Entity ent, ::Physics::MaterialType mat, const ::Physics::CapsuleCollider& c)
    {
      internalActor = ::Physics::PhysicsManager::AddStaticActorEntity(ent, mat, c);
    }

    ::Physics::StaticActorInterface Interface()
    {
      return internalActor;
    }

    physx::PxRigidStatic* internalActor;
  };

  struct CharacterController
  {
    CharacterController(Entity ent, ::Physics::MaterialType mat, const ::Physics::CapsuleCollider& c)
    {
      internalController = ::Physics::PhysicsManager::AddCharacterControllerEntity(ent, mat, c);
    }

    ::Physics::CharacterControllerInterface Interface()
    {
      return internalController;
    }

    physx::PxController* internalController;
  };
}