#include "PCH.h"
#include "Physics.h"
#include <engine/ecs/Entity.h>
#include <engine/ecs/component/Transform.h>
#include <engine/ecs/component/Physics.h>

#include <glm/gtx/quaternion.hpp>

#ifndef NDEBUG
#define _DEBUG
#endif

#include <PhysX/physx/include/PxPhysicsAPI.h>
#include <PhysX/physx/include/cooking/PxCooking.h>
#include <PhysX/pxshared/include/foundation/PxAllocatorCallback.h>
#include <PhysX/pxshared/include/foundation/PxErrorCallback.h>

#define PX_RELEASE(x)	if(x)	{ x->release(); x = NULL;	}
#define PVD_DEBUG 1
#define FIXED_STEP 1
#define ENABLE_GPU 1 // PhysX will automatically disable GPU simulation for non CUDA-compatible devices; this is just for debugging

constexpr double STEP = 1.0f / 50.0f;

using namespace physx;
namespace
{
  class PxLockRead
  {
  public:
    PxLockRead(PxScene* scn) : scene(scn) { scene->lockRead(); }
    ~PxLockRead() { scene->unlockRead(); }

  private:
    PxScene* scene;
  };
  class PxLockWrite
  {
  public:
    PxLockWrite(PxScene* scn) : scene(scn) { scene->lockWrite(); }
    ~PxLockWrite() { scene->unlockWrite(); }

  private:
    PxScene* scene;
  };

  PxVec3 toPxVec3(const glm::vec3& v)
  {
    return { v.x, v.y, v.z };
  }

  glm::vec3 toGlmVec3(const PxVec3& v)
  {
    return { v.x, v.y, v.z };
  }

  PxQuat toPxQuat(const glm::quat& q)
  {
    return { q.x, q.y, q.z, q.w }; // px quat constructor takes x, y, z, w
  }

  glm::quat toGlmQuat(const PxQuat& q)
  {
    return { q.w, q.x, q.y, q.z }; // glm quat constructor takes w, x, y, z
  }

  PxMat44 toPxMat4(const glm::mat4& m)
  {
    return
    {
      { m[0][0], m[0][1], m[0][2], m[0][3] },
      { m[1][0], m[1][1], m[1][2], m[1][3] },
      { m[2][0], m[2][1], m[2][2], m[2][3] },
      { m[3][0], m[3][1], m[3][2], m[3][3] }
    };
  }

  glm::mat4 toGlmMat4(const PxMat44& m)
  {
    return
    {
      { m[0][0], m[0][1], m[0][2], m[0][3] },
      { m[1][0], m[1][1], m[1][2], m[1][3] },
      { m[2][0], m[2][1], m[2][2], m[2][3] },
      { m[3][0], m[3][1], m[3][2], m[3][3] }
    };
  }

  PxFilterFlags contactReportFilterShader(
    PxFilterObjectAttributes attributes0, PxFilterData filterData0,
    PxFilterObjectAttributes attributes1, PxFilterData filterData1,
    PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize)
  {
    PX_UNUSED(attributes0);
    PX_UNUSED(attributes1);
    PX_UNUSED(filterData0);
    PX_UNUSED(filterData1);
    PX_UNUSED(constantBlockSize);
    PX_UNUSED(constantBlock);

    // all initial and persisting reports for everything, with per-point data
    pairFlags = PxPairFlag::eSOLVE_CONTACT
      | PxPairFlag::eDETECT_DISCRETE_CONTACT
      | PxPairFlag::eNOTIFY_TOUCH_FOUND
      | PxPairFlag::eNOTIFY_TOUCH_PERSISTS
      | PxPairFlag::eNOTIFY_CONTACT_POINTS
      | PxPairFlag::eDETECT_CCD_CONTACT;
    return PxFilterFlag::eDEFAULT;
  }


  class ErrorCallback : public PxDefaultErrorCallback
  {
  public:
    virtual void reportError(PxErrorCode::Enum code, const char* message, const char* file, int line)
    {
      PxDefaultErrorCallback::reportError(code, message, file, line);
    }
  };

  static PxDefaultAllocator gAllocator;
  static ErrorCallback gErrorCallback;
}


class ContactReportCallback : public PxSimulationEventCallback
{
public:
  ContactReportCallback(std::unordered_map<physx::PxRigidActor*, Entity>& ea) : gEntityActors(ea) {}

private:
  std::unordered_map<physx::PxRigidActor*, Entity>& gEntityActors;

  void onConstraintBreak(PxConstraintInfo* constraints, PxU32 count) { PX_UNUSED(constraints); PX_UNUSED(count); }
  void onWake(PxActor** actors, PxU32 count) { PX_UNUSED(actors); PX_UNUSED(count); }
  void onSleep(PxActor** actors, PxU32 count) { PX_UNUSED(actors); PX_UNUSED(count); }
  void onTrigger(PxTriggerPair* pairs, PxU32 count) { PX_UNUSED(pairs); PX_UNUSED(count); }
  void onAdvance(const PxRigidBody* const*, const PxTransform*, const PxU32) {}
  void onContact(const PxContactPairHeader& pairHeader, [[maybe_unused]] const PxContactPair* pairs, [[maybe_unused]] PxU32 nbPairs)
  {
    auto it1 = gEntityActors.find(pairHeader.actors[0]);
    auto it2 = gEntityActors.find(pairHeader.actors[1]);
    if (it1 != gEntityActors.end())
    {
      //printf("%s", it1->second.GetComponent<Components::Tag>().tag.c_str());
    }
    if (it2 != gEntityActors.end())
    {
      //printf("%s", it2->second.GetComponent<Components::Tag>().tag.c_str());
    }
  }
};

void Physics::PhysicsManager::Init()
{
  gContactReportCallback = new ContactReportCallback(gEntityActors);

  gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);
#if PVD_DEBUG
  gPvd = PxCreatePvd(*gFoundation);
  PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
  gPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);
#endif
  PxTolerancesScale tolerances;
  tolerances.length = 1;
  tolerances.speed = 15.81f;
  gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, tolerances, true, gPvd);
  gCooking = PxCreateCooking(PX_PHYSICS_VERSION, *gFoundation, PxCookingParams(tolerances));
  PxInitExtensions(*gPhysics, gPvd);
  gDispatcher = PxDefaultCpuDispatcherCreate(0);


  PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
  sceneDesc.cpuDispatcher = gDispatcher;
  sceneDesc.gravity = PxVec3(0, -15.81f, 0);
  sceneDesc.filterShader = contactReportFilterShader;
  sceneDesc.simulationEventCallback = gContactReportCallback;
  sceneDesc.solverType = PxSolverType::ePGS; // faster than eTGS
  sceneDesc.flags |= PxSceneFlag::eREQUIRE_RW_LOCK;

#if ENABLE_GPU
  PxCudaContextManagerDesc cudaContextManagerDesc;
  gCudaContextManager = PxCreateCudaContextManager(*gFoundation, cudaContextManagerDesc, PxGetProfilerCallback());
  sceneDesc.cudaContextManager = gCudaContextManager;
  sceneDesc.flags |= PxSceneFlag::eENABLE_GPU_DYNAMICS;
#endif

  gScene = gPhysics->createScene(sceneDesc);
  PxLockWrite lkw(gScene);
  gCManager = PxCreateControllerManager(*gScene);
  PxPvdSceneClient* pvdClient = gScene->getScenePvdClient();
  if (pvdClient)
  {
    pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
  }

  gMaterials = std::vector<PxMaterial*>(2);
  gMaterials[(int)MaterialType::PLAYER] = gPhysics->createMaterial(0.2f, 0.2f, -1.0f);
  gMaterials[(int)MaterialType::TERRAIN] = gPhysics->createMaterial(0.4f, 0.4f, .5f);

  PxRigidStatic* groundPlane = PxCreatePlane(*gPhysics, PxPlane(0, 1, 0, 0), *gMaterials[(int)MaterialType::TERRAIN]);
  gScene->addActor(*groundPlane);
}

void Physics::PhysicsManager::Shutdown()
{
  gScene->lockWrite();
  PX_RELEASE(gCManager);
  gScene->unlockWrite();
  PX_RELEASE(gScene);
  PX_RELEASE(gCudaContextManager);
  PX_RELEASE(gDispatcher);
  PxCloseExtensions();
  PX_RELEASE(gCooking);
  PX_RELEASE(gPhysics);
  if (gPvd)
  {
    PxPvdTransport* transport = gPvd->getTransport();
    PX_RELEASE(gPvd);
    PX_RELEASE(transport);
  }
  PX_RELEASE(gFoundation);

  delete gContactReportCallback;
}

void Physics::PhysicsManager::Simulate(Timestep timestep)
{
  // update character controllers every frame
  for (auto& [controller, entity] : gEntityControllers)
  {
    const auto& p = controller->getPosition();
    entity.GetComponent<Component::Transform>().SetTranslation({ p.x, p.y, p.z });
  }

  static bool resultsReady = true;
  bool asdf = false;
#if FIXED_STEP
  static double accumulator = 0;
  accumulator += timestep.dt_effective;
  accumulator = glm::min(accumulator, STEP * 20); // accumulate 20 steps of backlog
  if (accumulator > STEP) // NOTE: not while loop, because we want to avoid the Well of Despair
  {
    PxLockWrite lkw(gScene);
    if (resultsReady)
    {
      gScene->simulate(STEP);
      resultsReady = false;
    }
    if (gScene->fetchResults(false))
    {
      resultsReady = true;
      accumulator -= STEP;
      asdf = true;
    }
  }
#else
  gScene->simulate(dt);
  gScene->fetchResults(true);
  resultsReady = true;
#endif
  if (!asdf)
    return;

  gScene->lockRead();
  const int numstatic = gScene->getNbActors(PxActorTypeFlag::eRIGID_STATIC);
  //gScene->
  //printf("Static: %d\n", numstatic);

  // update all entity transforms whose actor counterpart was updated
  const auto actorTypes = PxActorTypeFlag::eRIGID_DYNAMIC | PxActorTypeFlag::eRIGID_STATIC;
  const auto numActors = gScene->getNbActors(actorTypes);
  if (numActors > 0)
  {
    std::vector<PxRigidActor*> actors(numActors);
    gScene->getActors(actorTypes, reinterpret_cast<PxActor**>(actors.data()), numActors);
    for (auto actor : actors)
    {
      const bool sleeping = actor->is<PxRigidDynamic>() ? actor->is<PxRigidDynamic>()->isSleeping() : false;
      if (sleeping)
        continue;
      const auto& pose = actor->getGlobalPose();
      auto entityit = gEntityActors.find(actor);
      if (entityit != gEntityActors.end())
      {
        auto& tr = entityit->second.GetComponent<Component::Transform>();
        auto* interp = entityit->second.TryGetComponent<Component::InterpolatedPhysics>();
        auto* dynamic = actor->is<PxRigidDynamic>();
        if (interp && dynamic)
        {
          interp->timeSinceUpdate = 0;
          //interp->linearVelocity = toGlmVec3(dynamic->getLinearVelocity());
          //interp->angularVelocity = toGlmVec3(dynamic->getAngularVelocity());
          interp->prevPos = tr.GetTranslation();
          interp->prevRot = tr.GetRotation();
        }

        // an entity with physics must have a transform
        tr.SetTranslation(toGlmVec3(pose.p));
        glm::quat q(toGlmQuat(pose.q));
        tr.SetRotation(q);
      }
    }
  }
  gScene->unlockRead();
}

double Physics::PhysicsManager::GetStep()
{
  return STEP;
}

physx::PxRigidDynamic* Physics::PhysicsManager::AddDynamicActorEntity(Entity entity, MaterialType material, BoxCollider collider, DynamicActorFlags flags)
{
  const auto& tr = entity.GetComponent<Component::Transform>();
  //collider.halfExtents *= tr.GetScale();
  glm::quat q(tr.GetRotation());
  glm::vec3 pos(tr.GetTranslation());
  PxTransform tr2(toPxVec3(pos), toPxQuat(q));
  tr2.isValid();
  PxBoxGeometry geom(toPxVec3(collider.halfExtents));
  PxRigidDynamic* dynamic = PxCreateDynamic(*gPhysics, tr2, geom, *gMaterials[(int)material], 10.0f);
  dynamic->setAngularDamping(0.5f);
  dynamic->setRigidBodyFlags((PxRigidBodyFlags)(uint32_t)flags);
  gEntityActors[dynamic] = entity;
  PxLockWrite lkw(gScene);
  gScene->addActor(*dynamic);
  return dynamic;
}

physx::PxRigidDynamic* Physics::PhysicsManager::AddDynamicActorEntity(Entity entity, MaterialType material, CapsuleCollider collider, DynamicActorFlags flags)
{
  const auto& tr = entity.GetComponent<Component::Transform>();
  glm::quat q(tr.GetRotation());
  glm::vec3 pos(tr.GetTranslation());
  PxTransform tr2(toPxVec3(pos), toPxQuat(q));
  PxCapsuleGeometry geom(collider.radius, collider.halfHeight);
  PxRigidDynamic* dynamic = PxCreateDynamic(*gPhysics, tr2, geom, *gMaterials[(int)material], 10.0f);
  dynamic->setAngularDamping(0.5f);
  dynamic->setRigidBodyFlags((PxRigidBodyFlags)(uint32_t)flags);
  gEntityActors[dynamic] = entity;
  PxLockWrite lkw(gScene);
  gScene->addActor(*dynamic);
  return dynamic;
}

physx::PxRigidStatic* Physics::PhysicsManager::AddStaticActorEntity(Entity entity, MaterialType material, BoxCollider collider)
{
  const auto& tr = entity.GetComponent<Component::Transform>();
  glm::quat q(tr.GetRotation());
  glm::vec3 pos(tr.GetTranslation());
  PxTransform tr2(toPxVec3(pos), toPxQuat(q));
  PxBoxGeometry geom(toPxVec3(collider.halfExtents));
  PxRigidStatic* pStatic = PxCreateStatic(*gPhysics, tr2, geom, *gMaterials[(int)material]);
  gEntityActors[pStatic] = entity;
  PxLockWrite lkw(gScene);
  gScene->addActor(*pStatic);
  return pStatic;
}

physx::PxRigidStatic* Physics::PhysicsManager::AddStaticActorEntity(Entity entity, MaterialType material, CapsuleCollider collider)
{
  const auto& tr = entity.GetComponent<Component::Transform>();
  glm::quat q(tr.GetRotation());
  glm::vec3 pos(tr.GetTranslation());
  PxTransform tr2(toPxVec3(pos), toPxQuat(q));
  PxCapsuleGeometry geom(collider.radius, collider.halfHeight);
  PxRigidStatic* pStatic = PxCreateStatic(*gPhysics, tr2, geom, *gMaterials[(int)material]);
  gEntityActors[pStatic] = entity;
  PxLockWrite lkw(gScene);
  gScene->addActor(*pStatic);
  return pStatic;
}

void Physics::PhysicsManager::RemoveActorEntity(physx::PxRigidActor* actor)
{
  if (actor)
  {
    gEntityActors.erase(actor);
    PxLockWrite lkw(gScene);
    gScene->removeActor(*actor);
  }
}

physx::PxRigidStatic* Physics::PhysicsManager::AddStaticActorGeneric(MaterialType material, const MeshCollider& collider, const glm::mat4& transform)
{
  ASSERT(collider.indices.size() % 3 == 0);
  if (collider.indices.size() == 0)
    return nullptr;

  PxTolerancesScale scale;
  PxCookingParams params(scale);
  // disable mesh cleaning - perform mesh validation on development configurations
  params.meshPreprocessParams |= PxMeshPreprocessingFlag::eDISABLE_CLEAN_MESH;
  // disable edge precompute, edges are set for each triangle, slows contact generation
  params.meshPreprocessParams |= PxMeshPreprocessingFlag::eDISABLE_ACTIVE_EDGES_PRECOMPUTE;
  //params.meshPreprocessParams |= PxMeshPreprocessingFlag::eWELD_VERTICES;
  params.meshWeldTolerance = .1f;
  // lower hierarchy for internal mesh
  //params.meshCookingHint = PxMeshCookingHint::eCOOKING_PERFORMANCE;

  gCooking->setParams(params);

  PxTriangleMeshDesc meshDesc;
  meshDesc.points.count = static_cast<PxU32>(collider.vertices.size());
  meshDesc.points.stride = sizeof(glm::vec3);
  meshDesc.points.data = collider.vertices.data();

  meshDesc.triangles.count = static_cast<PxU32>(collider.indices.size() / 3);
  meshDesc.triangles.stride = 3 * sizeof(uint32_t);
  meshDesc.triangles.data = collider.indices.data();

#ifdef _DEBUG
  // mesh should be validated before cooked without the mesh cleaning
  //bool res = gCooking->validateTriangleMesh(meshDesc);
  //ASSERT(res);
#endif
  PxTriangleMesh* aTriangleMesh = gCooking->createTriangleMesh(meshDesc,
    gPhysics->getPhysicsInsertionCallback());
  PxTriangleMeshGeometry geom(aTriangleMesh);
  PxRigidStatic* pStatic = PxCreateStatic(*gPhysics, PxTransform(toPxMat4(transform)), geom, *gMaterials[(int)material]);

  gGenericActors.insert(pStatic);
  PxLockWrite lkw(gScene);
  gScene->addActor(*pStatic);
  aTriangleMesh->release();
  return pStatic;
}

void Physics::PhysicsManager::RemoveActorGeneric(physx::PxRigidActor* actor)
{
  if (actor)
  {
    // TODO: figure out how to actually free the thingy because this makes a memory leak
    gGenericActors.erase(actor);
    PxLockWrite lkw(gScene);
    gScene->removeActor(*actor);
    PX_RELEASE(actor);
  }
}

physx::PxController* Physics::PhysicsManager::AddCharacterControllerEntity(Entity entity, MaterialType material, CapsuleCollider collider)
{
  PxCapsuleControllerDesc desc;
  desc.upDirection = { 0, 1, 0 };
  desc.density = 10.f;
  desc.stepOffset = .1f;
  desc.material = gMaterials[(int)material];
  desc.height = 2.f * collider.halfHeight;
  desc.radius = collider.radius;
  desc.contactOffset = .01f;

  PxLockWrite lkw(&gCManager->getScene());
  PxController* controller = gCManager->createController(desc);
  auto p = entity.GetComponent<Component::Transform>().GetTranslation();
  controller->setPosition({ p.x, p.y, p.z });
  gEntityControllers[controller] = entity;
  //PxShape* sh = nullptr;
  //controller->getActor()->getShapes(&sh, 1);
  //sh->setSimulationFilterData(PxFilterData());
  return controller;
}

void Physics::PhysicsManager::RemoveCharacterControllerEntity(physx::PxController* controller)
{
  if (controller)
  {
    PxLockWrite lkw(controller->getScene());
    gEntityControllers.erase(controller);
    controller->release();
  }
}

void Physics::DynamicActorInterface::AddForce(const glm::vec3& force, ForceMode mode)
{
  PxLockWrite lkw(actor->getScene());
  actor->addForce(toPxVec3(force), (PxForceMode::Enum)mode);
}

void Physics::DynamicActorInterface::SetPosition(const glm::vec3& pos)
{
  auto pose = actor->getGlobalPose();
  pose.p = toPxVec3(pos);
  PxLockWrite lkw(actor->getScene());
  actor->setGlobalPose(pose);
}

glm::vec3 Physics::DynamicActorInterface::GetVelocity()
{
  PxLockRead lkr(actor->getScene());
  return toGlmVec3(actor->getLinearVelocity());
}

void Physics::DynamicActorInterface::SetVelocity(const glm::vec3& vel)
{
  PxLockWrite lkw(actor->getScene());
  actor->setLinearVelocity(toPxVec3(vel));
}

void Physics::DynamicActorInterface::SetMaxVelocity(float vel)
{
  PxLockWrite lkw(actor->getScene());
  actor->setMaxLinearVelocity(vel);
}

void Physics::DynamicActorInterface::SetLockFlags(LockFlags flags)
{
  PxLockWrite lkw(actor->getScene());
  actor->setRigidDynamicLockFlags((PxRigidDynamicLockFlag::Enum)(uint32_t)flags);
}

void Physics::DynamicActorInterface::SetActorFlags(ActorFlags flags)
{
  PxLockWrite lkw(actor->getScene());
  actor->setActorFlags((PxActorFlags)(uint32_t)flags);
}

void Physics::DynamicActorInterface::SetMass(float mass)
{
  PxLockWrite lkw(actor->getScene());
  actor->setMass(mass);
}

Physics::ControllerCollisionFlags Physics::CharacterControllerInterface::Move(const glm::vec3& disp, float dt)
{
  PxLockWrite lkw(controller->getScene());
  PxControllerFilters filters;
  return (ControllerCollisionFlags)controller->move(toPxVec3(disp), .0001f, dt, filters);
}

glm::vec3 Physics::CharacterControllerInterface::GetPosition()
{
  PxLockRead lkr(controller->getScene());
  // physx has to make it a PITA so it returns a dvec3 instead of a vec3 like a normal person would
  const auto& p = controller->getPosition();
  return { p.x, p.y, p.z };
}
