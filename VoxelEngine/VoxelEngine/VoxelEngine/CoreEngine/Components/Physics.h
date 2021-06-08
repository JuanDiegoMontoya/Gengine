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

    //DynamicPhysics(DynamicPhysics&& rhs) noexcept { *this = std::move(rhs); }
    //DynamicPhysics& operator=(DynamicPhysics&& rhs) noexcept
    //{
    //  if (&rhs == this) return *this;
    //  internalActor = std::exchange(rhs.internalActor, nullptr);
    //  return *this;
    //}
    //DynamicPhysics(const DynamicPhysics&) = delete;
    //DynamicPhysics& operator=(const DynamicPhysics&) = delete;

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

    StaticPhysics(StaticPhysics&& rhs) noexcept { *this = std::move(rhs); }
    StaticPhysics& operator=(StaticPhysics&& rhs) noexcept
    {
      internalActor = std::exchange(rhs.internalActor, nullptr);
      return *this;
    }
    StaticPhysics(const StaticPhysics&) = delete;
    StaticPhysics& operator=(const StaticPhysics&) = delete;

    ::Physics::StaticActorInterface Interface()
    {
      return internalActor;
    }

    //~StaticPhysics()
    //{
    //  if (internalActor)
    //  {
    //    //printf("PHYSICS %p really DELETED\n", this);
    //    ::Physics::PhysicsManager::RemoveActorEntity(reinterpret_cast<physx::PxRigidActor*>(internalActor));
    //  }
    //}

  //private:
    physx::PxRigidStatic* internalActor;
  };

  struct CharacterController
  {
    //CharacterController(Entity ent, ::Physics::MaterialType mat, const ::Physics::BoxCollider& c)
    //{
    //  internalController = ::Physics::PhysicsManager::AddCharacterControllerEntity(ent, mat, c);
    //}
    CharacterController(Entity ent, ::Physics::MaterialType mat, const ::Physics::CapsuleCollider& c)
    {
      internalController = ::Physics::PhysicsManager::AddCharacterControllerEntity(ent, mat, c);
    }

    //CharacterController(CharacterController&& rhs) noexcept { *this = std::move(rhs); }
    //CharacterController& operator=(CharacterController&& rhs) noexcept
    //{
    //  internalController = std::exchange(rhs.internalController, nullptr);
    //  return *this;
    //}
    //CharacterController(const CharacterController&) = delete;
    //CharacterController& operator=(const CharacterController&) = delete;

    ::Physics::CharacterControllerInterface Interface()
    {
      return internalController;
    }

    //~CharacterController()
    //{
    //  if (internalController)
    //  {
    //    //printf("PHYSICS %p really DELETED\n", this);
    //    ::Physics::PhysicsManager::RemoveCharacterControllerEntity(internalController);
    //  }
    //}

  //private:
    physx::PxController* internalController;
  };
}