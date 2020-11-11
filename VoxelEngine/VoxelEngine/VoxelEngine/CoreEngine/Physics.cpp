#include <CoreEngine/Physics.h>
#include <CoreEngine/Entity.h>
#include <CoreEngine/Components.h>

#include <glm/gtx/quaternion.hpp>

#ifndef NDEBUG
#define _DEBUG
#endif
#include <PxPhysicsAPI.h>
#include <cooking/PxCooking.h>
#include <foundation/PxAllocatorCallback.h>
#include <foundation/PxErrorCallback.h>

#define PX_RELEASE(x)	if(x)	{ x->release(); x = NULL;	}
#define PVD_DEBUG 1
#define FIXED_STEP 1

#pragma optimize("", off)

using namespace physx;
namespace
{
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

  class ContactReportCallback : public PxSimulationEventCallback
  {
    void onConstraintBreak(PxConstraintInfo* constraints, PxU32 count) { PX_UNUSED(constraints); PX_UNUSED(count); }
    void onWake(PxActor** actors, PxU32 count) { PX_UNUSED(actors); PX_UNUSED(count); }
    void onSleep(PxActor** actors, PxU32 count) { PX_UNUSED(actors); PX_UNUSED(count); }
    void onTrigger(PxTriggerPair* pairs, PxU32 count) { PX_UNUSED(pairs); PX_UNUSED(count); }
    void onAdvance(const PxRigidBody* const*, const PxTransform*, const PxU32) {}
    void onContact(const PxContactPairHeader& pairHeader, const PxContactPair* pairs, PxU32 nbPairs)
    {
      PX_UNUSED((pairHeader));
      std::vector<PxContactPairPoint> contactPoints;

      for (PxU32 i = 0; i < nbPairs; i++)
      {
        PxU32 contactCount = pairs[i].contactCount;
        if (contactCount)
        {
          contactPoints.resize(contactCount);
          pairs[i].extractContacts(&contactPoints[0], contactCount);

          for (PxU32 j = 0; j < contactCount; j++)
          {
            //gContactPositions.push_back(contactPoints[j].position);
            //gContactImpulses.push_back(contactPoints[j].impulse);
          }
        }
      }
    }
  };

  static ContactReportCallback gContactReportCallback;

  static PxDefaultAllocator gAllocator;
  static PxDefaultErrorCallback gErrorCallback;


  //PxRigidDynamic* createDynamic(const PxTransform& t, const PxGeometry& geometry, const PxVec3& velocity = PxVec3(0))
  //{
  //	PxRigidDynamic* dynamic = PxCreateDynamic(*gPhysics, t, geometry, *gMaterial, 10.0f);
  //	dynamic->setAngularDamping(0.5f);
  //	dynamic->setLinearVelocity(velocity);
  //	gScene->addActor(*dynamic);
  //	return dynamic;
  //}
}

void Physics::PhysicsManager::Init()
{
  gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);
#if PVD_DEBUG
  gPvd = PxCreatePvd(*gFoundation);
  PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
  gPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);
#endif
  PxTolerancesScale tolerances;
  tolerances.length = 1;
  tolerances.speed = 15.81;
  gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, tolerances, true, gPvd);
  gCooking = PxCreateCooking(PX_PHYSICS_VERSION, *gFoundation, PxCookingParams(tolerances));
  PxInitExtensions(*gPhysics, gPvd);
  gDispatcher = PxDefaultCpuDispatcherCreate(0);
  PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
  sceneDesc.cpuDispatcher = gDispatcher;
  sceneDesc.gravity = PxVec3(0, -15.81f, 0);
  sceneDesc.filterShader = contactReportFilterShader;
  sceneDesc.simulationEventCallback = &gContactReportCallback;
  sceneDesc.solverType = PxSolverType::eTGS;
  sceneDesc.flags |= PxSceneFlag::eENABLE_CCD;
  gScene = gPhysics->createScene(sceneDesc);
  gCManager = PxCreateControllerManager(*gScene);

  PxPvdSceneClient* pvdClient = gScene->getScenePvdClient();
  if (pvdClient)
  {
    pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
  }

  gMaterials = std::vector<PxMaterial*>(2);
  gMaterials[(int)MaterialType::Player] = gPhysics->createMaterial(0.2f, 0.2f, -1.0f);
  gMaterials[(int)MaterialType::Terrain] = gPhysics->createMaterial(0.4f, 0.4f, .5f);

  PxRigidStatic* groundPlane = PxCreatePlane(*gPhysics, PxPlane(0, 1, 0, 0), *gMaterials[(int)MaterialType::Terrain]);
  gScene->addActor(*groundPlane);
}

void Physics::PhysicsManager::Shutdown()
{
  //for (auto [actor, Entity] : gEntityActors)
  //  actor->release();
  //for (auto actor : gGenericActors)
  //  actor->release();
  //for (auto* mat : gMaterials)
  //  mat->release();

  PX_RELEASE(gCManager);
  PX_RELEASE(gScene);
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
}

void Physics::PhysicsManager::Simulate(float dt)
{
  // update character controllers every frame
  for (auto [controller, entity] : gEntityControllers)
  {
    auto p = controller->getPosition();
    entity.GetComponent<Components::Transform>().SetTranslation({ p.x, p.y, p.z });
  }

  static bool resultsReady = true;
#if FIXED_STEP
  static float accumulator = 0;
  static const float step = 1.0f / 60.0f;
  accumulator += dt;
  accumulator = glm::min(accumulator, step * 20); // accumulate 20 steps of backlog
  if (accumulator > step) // NOTE: not while loop, because we want to avoid the Well of Despair
  {
    if (resultsReady)
    {
      gScene->simulate(step);
      resultsReady = false;
    }
    if (gScene->fetchResults(false))
    {
      resultsReady = true;
      accumulator -= step;
    }
  }
#else
  gScene->simulate(dt);
  gScene->fetchResults(true);
  resultsReady = true;
#endif
  if (!resultsReady)
    return;
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
      Entity entity = gEntityActors[actor];
      if (entity)
      {
        // an entity with physics must have a transform
        auto& tr = entity.GetComponent<Components::Transform>();
        tr.SetTranslation(toGlmVec3(pose.p));
        glm::quat q(toGlmQuat(pose.q));
        //tr.SetRotation(*(glm::quat*)&pose.q);
        tr.SetRotation(q);
      }
    }
  }
}

physx::PxRigidDynamic* Physics::PhysicsManager::AddDynamicActorEntity(Entity entity, MaterialType material, BoxCollider collider, DynamicActorFlags flags)
{
  const auto& tr = entity.GetComponent<Components::Transform>();
  //collider.halfExtents *= tr.GetScale();
  glm::quat q(tr.GetRotation());
  glm::vec3 pos(tr.GetTranslation());
  PxTransform tr2(toPxVec3(pos), toPxQuat(q));
  PxBoxGeometry geom(toPxVec3(collider.halfExtents));
  PxRigidDynamic* dynamic = PxCreateDynamic(*gPhysics, tr2, geom, *gMaterials[(int)material], 10.0f);
  dynamic->setAngularDamping(0.5f);
  dynamic->setRigidBodyFlags((PxRigidBodyFlags)flags);
  gEntityActors[dynamic] = entity;
  gScene->addActor(*dynamic);
  return dynamic;
}

physx::PxRigidDynamic* Physics::PhysicsManager::AddDynamicActorEntity(Entity entity, MaterialType material, CapsuleCollider collider, DynamicActorFlags flags)
{
  const auto& tr = entity.GetComponent<Components::Transform>();
  glm::quat q(tr.GetRotation());
  glm::vec3 pos(tr.GetTranslation());
  PxTransform tr2(toPxVec3(pos), toPxQuat(q));
  PxCapsuleGeometry geom(collider.radius, collider.halfHeight);
  PxRigidDynamic* dynamic = PxCreateDynamic(*gPhysics, tr2, geom, *gMaterials[(int)material], 10.0f);
  dynamic->setAngularDamping(0.5f);
  dynamic->setRigidBodyFlags((PxRigidBodyFlags)flags);
  gEntityActors[dynamic] = entity;
  gScene->addActor(*dynamic);
  return dynamic;
}

physx::PxRigidStatic* Physics::PhysicsManager::AddStaticActorEntity(Entity entity, MaterialType material, BoxCollider collider)
{
  const auto& tr = entity.GetComponent<Components::Transform>();
  glm::quat q(tr.GetRotation());
  glm::vec3 pos(tr.GetTranslation());
  PxTransform tr2(toPxVec3(pos), toPxQuat(q));
  PxBoxGeometry geom(toPxVec3(collider.halfExtents));
  PxRigidStatic* pStatic = PxCreateStatic(*gPhysics, tr2, geom, *gMaterials[(int)material]);
  gEntityActors[pStatic] = entity;
  gScene->addActor(*pStatic);
  return pStatic;
}

physx::PxRigidStatic* Physics::PhysicsManager::AddStaticActorEntity(Entity entity, MaterialType material, CapsuleCollider collider)
{
  const auto& tr = entity.GetComponent<Components::Transform>();
  glm::quat q(tr.GetRotation());
  glm::vec3 pos(tr.GetTranslation());
  PxTransform tr2(toPxVec3(pos), toPxQuat(q));
  PxCapsuleGeometry geom(collider.radius, collider.halfHeight);
  PxRigidStatic* pStatic = PxCreateStatic(*gPhysics, tr2, geom, *gMaterials[(int)material]);
  gEntityActors[pStatic] = entity;
  gScene->addActor(*pStatic);
  return pStatic;
}

void Physics::PhysicsManager::RemoveActorEntity(physx::PxRigidActor* actor)
{
  if (actor)
  {
    gScene->removeActor(*actor);
    gEntityActors.erase(actor);
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
  //params.meshPreprocessParams |= PxMeshPreprocessingFlag::eDISABLE_CLEAN_MESH;
  // disable edge precompute, edges are set for each triangle, slows contact generation
  //params.meshPreprocessParams |= PxMeshPreprocessingFlag::eDISABLE_ACTIVE_EDGES_PRECOMPUTE;
  params.meshPreprocessParams |= PxMeshPreprocessingFlag::eWELD_VERTICES;
  params.meshWeldTolerance = .1f;
  // lower hierarchy for internal mesh
  //params.meshCookingHint = PxMeshCookingHint::eCOOKING_PERFORMANCE;
  
  gCooking->setParams(params);

  PxTriangleMeshDesc meshDesc;
  meshDesc.points.count = collider.vertices.size();
  meshDesc.points.stride = sizeof(glm::vec3);
  meshDesc.points.data = collider.vertices.data();

  meshDesc.triangles.count = collider.indices.size() / 3;
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
  gScene->addActor(*pStatic);
  return pStatic;
}

void Physics::PhysicsManager::RemoveActorGeneric(physx::PxRigidActor* actor)
{
  if (actor)
  {
    gGenericActors.erase(actor);
    gScene->removeActor(*actor);
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
  desc.contactOffset = .01;

  PxController* controller = gCManager->createController(desc);
  auto p = entity.GetComponent<Components::Transform>().GetTranslation();
  controller->setPosition({ p.x, p.y, p.z });
  gEntityControllers[controller] = entity;
  return controller;
}

void Physics::PhysicsManager::RemoveCharacterControllerEntity(physx::PxController* controller)
{
  if (controller)
  {
    gEntityControllers.erase(controller);
    controller->release();
  }
}

void Physics::DynamicActorInterface::AddForce(const glm::vec3& force, ForceMode mode)
{
  actor->addForce(toPxVec3(force), (PxForceMode::Enum)mode);
}

void Physics::DynamicActorInterface::SetPosition(const glm::vec3& pos)
{
  auto pose = actor->getGlobalPose();
  pose.p = toPxVec3(pos);
  actor->setGlobalPose(pose);
}

glm::vec3 Physics::DynamicActorInterface::GetVelocity()
{
  return toGlmVec3(actor->getLinearVelocity());
}

void Physics::DynamicActorInterface::SetVelocity(const glm::vec3& vel)
{
  actor->setLinearVelocity(toPxVec3(vel));
}

void Physics::DynamicActorInterface::SetMaxVelocity(float vel)
{
  actor->setMaxLinearVelocity(vel);
}

void Physics::DynamicActorInterface::SetLockFlags(LockFlags flags)
{
  actor->setRigidDynamicLockFlags((PxRigidDynamicLockFlag::Enum)flags);
}

void Physics::DynamicActorInterface::SetActorFlags(ActorFlags flags)
{
  actor->setActorFlags((PxActorFlags)flags);
}

void Physics::DynamicActorInterface::SetMass(float mass)
{
  actor->setMass(mass);
}

Physics::ControllerCollisionFlags Physics::CharacterControllerInterface::Move(const glm::vec3& disp, float dt)
{
  PxControllerFilters filters;
  return (ControllerCollisionFlags)controller->move(toPxVec3(disp), .0001f, dt, filters);
}

glm::vec3 Physics::CharacterControllerInterface::GetPosition()
{
  // physx has to make it a PITA so it returns a dvec3 instead of a vec3 like a normal person would
  const auto& p = controller->getPosition();
  return { p.x, p.y, p.z };
}
