#pragma once
#include <glm/glm.hpp>
#include <CoreEngine/Entity.h>
#include <memory>

namespace physx
{
  class PxFoundation;
  class PxPhysics;
  class PxDefaultCpuDispatcher;
  class PxScene;
  class PxMaterial;
  class PxPvd;

  class PxRigidActor;
  class PxRigidDynamic;
}

namespace Components
{
  struct Physics;
}

namespace Physics
{
  struct BoxCollider
  {
    BoxCollider(const glm::vec3& halfextents) : halfExtents(halfextents) {}
    glm::vec3 halfExtents;
  };
  struct CapsuleCollider
  {
    CapsuleCollider(float r, float h) : radius(r), halfHeight(h) {}
    float radius;
    float halfHeight;
  };

  enum class MaterialType
  {
    Player,
    Terrain
  };

  enum class ForceMode
  {
    Force,
    Impulse,
    VelocityChange,
    Acceleration,
  };

  enum ActorFlag
  {
    DisableGravity = (1 << 1),
    DisableSimulation = (1 << 3),
  };
  using ActorFlags = int;

  enum LockFlag
  {
    LOCK_LINEAR_X = (1 << 0),
    LOCK_LINEAR_Y = (1 << 1),
    LOCK_LINEAR_Z = (1 << 2),
    LOCK_ANGULAR_X = (1 << 3),
    LOCK_ANGULAR_Y = (1 << 4),
    LOCK_ANGULAR_Z = (1 << 5)
  };
  using LockFlags = int;

  class PhysicsManager
  {
  public:
    static void Init();
    static void Shutdown();
    static void Simulate(float dt);

    static physx::PxRigidDynamic* AddActor(Entity entity, MaterialType material, BoxCollider collider);
    static physx::PxRigidDynamic* AddActor(Entity entity, MaterialType material, CapsuleCollider collider);
    static void RemoveActor(physx::PxRigidDynamic* actor);

  private:
    static inline physx::PxFoundation* gFoundation = nullptr;
    static inline physx::PxPhysics* gPhysics = nullptr;
    
    static inline physx::PxDefaultCpuDispatcher* gDispatcher = nullptr;
    static inline physx::PxScene* gScene = nullptr;
    static inline std::vector<physx::PxMaterial*> gMaterials;
    static inline physx::PxPvd* gPvd = nullptr;

    static inline std::unordered_map<physx::PxRigidDynamic*, Entity> gActors;
  };

  // allows public access to methods of doing stuff with actors
  class DynamicActorInterface
  {
  public:
    DynamicActorInterface(physx::PxRigidDynamic* atr) : actor(atr) {}
    void AddForce(const glm::vec3& force, ForceMode mode = ForceMode::Impulse);
    void SetPosition(const glm::vec3& pos);
    glm::vec3 GetVelocity();
    void SetVelocity(const glm::vec3& vel);
    void SetMaxVelocity(float vel);
    void SetLockFlags(LockFlags flags);
    void SetActorFlags(ActorFlags flags);

  private:
    physx::PxRigidDynamic* actor;
  };
}