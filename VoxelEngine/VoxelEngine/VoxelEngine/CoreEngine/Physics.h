#pragma once
#include <glm/glm.hpp>
#include <CoreEngine/Entity.h>
#include <memory>
#include "Flags.h"

namespace physx
{
  class PxFoundation;
  class PxPhysics;
  class PxDefaultCpuDispatcher;
  class PxScene;
  class PxMaterial;
  class PxPvd;
  class PxCooking;
  class PxControllerManager;
  class PxController;

  class PxRigidActor;
  class PxRigidDynamic;
  class PxRigidStatic;

  class PxCudaContextManager;
}
class ContactReportCallback;

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
  struct MeshCollider
  {
    std::vector<glm::vec3> vertices;
    std::vector<uint32_t> indices;
  };

  enum class MaterialType
  {
    PLAYER,
    TERRAIN
  };

  enum class ForceMode
  {
    FORCE,
    IMPULSE,
    VELOCITY_CHANGE,
    ACCELERATION,
  };

  enum class DynamicActorFlag
  {
    KINEMATIC = (1 << 0),
    ENABLE_CCD = (1 << 2),

  };
  DECLARE_FLAG_TYPE(DynamicActorFlags, DynamicActorFlag, uint32_t)

  enum class ActorFlag
  {
    DISABLE_GRAVITY = (1 << 1),
    DISABLE_SIMULATION = (1 << 3),
  };
  DECLARE_FLAG_TYPE(ActorFlags, ActorFlag, uint32_t)

  enum class LockFlag
  {
    LOCK_LINEAR_X = (1 << 0),
    LOCK_LINEAR_Y = (1 << 1),
    LOCK_LINEAR_Z = (1 << 2),
    LOCK_ANGULAR_X = (1 << 3),
    LOCK_ANGULAR_Y = (1 << 4),
    LOCK_ANGULAR_Z = (1 << 5)
  };
  DECLARE_FLAG_TYPE(LockFlags, LockFlag, uint32_t)

  enum class ControllerCollisionFlag
  {
    COLLISION_SIDES = (1 << 0),
    COLLISION_UP = (1 << 1),
    COLLISION_DOWN = (1 << 2)
  };
  DECLARE_FLAG_TYPE(ControllerCollisionFlags, ControllerCollisionFlag, uint32_t)

  enum class CollisionType : uint8_t
  {
    // trace types (no object can actually be one of these types)
    VISIBILITY,
    CAMERA,

    // object types
    WORLD_STATIC,
    WORLD_DYNAMIC,
    PLAYER,
    PHYSICS_BODY,
    DESTRUCTIBLE,

    CTCount_, // don't actually use this pls
  };

  enum class CollisionResponse : uint8_t
  {
    Ignore,
    Overlap,
    Block
  };

  //using CollisionResponseFlags = uint16_t;
  struct CollisionResponseFlags
  {

  private:
    uint16_t flags;
  };

  static_assert(sizeof(CollisionResponseFlags) * 8 >= (uint16_t)CollisionType::CTCount_ * 2,
    "Too many object types to fit within the size of CollisionResponseFlags. Consider \
    increasing the width of the underlying type.");

  class PhysicsManager
  {
  public:
    static void Init();
    static void Shutdown();
    static void Simulate(float dt);

    static physx::PxRigidDynamic* AddDynamicActorEntity(Entity entity, MaterialType material, BoxCollider collider, DynamicActorFlags flags);
    static physx::PxRigidDynamic* AddDynamicActorEntity(Entity entity, MaterialType material, CapsuleCollider collider, DynamicActorFlags flags);
    static physx::PxRigidStatic* AddStaticActorEntity(Entity entity, MaterialType material, BoxCollider collider);
    static physx::PxRigidStatic* AddStaticActorEntity(Entity entity, MaterialType material, CapsuleCollider collider);
    static void RemoveActorEntity(physx::PxRigidActor* actor);

    static physx::PxRigidStatic* AddStaticActorGeneric(MaterialType material, const MeshCollider& collider, const glm::mat4& transform);
    static void RemoveActorGeneric(physx::PxRigidActor* actor);

    static physx::PxController* AddCharacterControllerEntity(Entity entity, MaterialType material, CapsuleCollider collider);
    static void RemoveCharacterControllerEntity(physx::PxController* controller);

  private:
    static inline physx::PxFoundation* gFoundation = nullptr;
    static inline physx::PxPhysics* gPhysics = nullptr;

    static inline ContactReportCallback* gContactReportCallback;

    static inline physx::PxDefaultCpuDispatcher* gDispatcher = nullptr;
    static inline physx::PxScene* gScene = nullptr;
    static inline std::vector<physx::PxMaterial*> gMaterials;
    static inline physx::PxPvd* gPvd = nullptr;
    static inline physx::PxCooking* gCooking = nullptr;
    static inline physx::PxControllerManager* gCManager = nullptr;

    static inline physx::PxCudaContextManager* gCudaContextManager = nullptr;

    static inline std::unordered_map<physx::PxRigidActor*, Entity> gEntityActors;
    static inline std::unordered_map<physx::PxController*, Entity> gEntityControllers;
    static inline std::unordered_set<physx::PxRigidActor*> gGenericActors;
  };

  // allows public access to methods for doing stuff with dynamic actors (non-owning)
  class DynamicActorInterface
  {
  public:
    DynamicActorInterface(physx::PxRigidDynamic* atr) : actor(atr) {}
    void AddForce(const glm::vec3& force, ForceMode mode = ForceMode::IMPULSE);
    void SetPosition(const glm::vec3& pos);
    glm::vec3 GetVelocity();
    void SetVelocity(const glm::vec3& vel);
    void SetMaxVelocity(float vel);
    void SetLockFlags(LockFlags flags);
    void SetActorFlags(ActorFlags flags);
    void SetMass(float mass);

  private:
    physx::PxRigidDynamic* actor;
  };

  // allows public access to methods for doing stuff with static actors (non-owning)
  class StaticActorInterface
  {
  public:
    StaticActorInterface(physx::PxRigidStatic* atr) : actor(atr) {}

  private:
    physx::PxRigidStatic* actor;
  };

  class CharacterControllerInterface
  {
  public:
    CharacterControllerInterface(physx::PxController* cont) : controller(cont) {}
    ControllerCollisionFlags Move(const glm::vec3& disp, float dt);
    glm::vec3 GetPosition();

  private:
    physx::PxController* controller;
  };
}