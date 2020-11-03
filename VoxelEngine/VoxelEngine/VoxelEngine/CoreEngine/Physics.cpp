#include <CoreEngine/Physics.h>
#include <CoreEngine/Entity.h>
#include <CoreEngine/Components.h>

#include <glm/gtx/quaternion.hpp>

#ifndef NDEBUG
#define _DEBUG
#endif
#include <PxPhysicsAPI.h>
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
    return { q.x, q.y, q.z, q.w };
  }

  glm::quat toGlmQuat(const PxQuat& q)
  {
    return { q.x, q.y, q.z, q.w };
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
    pairFlags = PxPairFlag::eSOLVE_CONTACT | PxPairFlag::eDETECT_DISCRETE_CONTACT
      | PxPairFlag::eNOTIFY_TOUCH_FOUND
      | PxPairFlag::eNOTIFY_TOUCH_PERSISTS
      | PxPairFlag::eNOTIFY_CONTACT_POINTS;
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
  tolerances.speed = 9.81;
  gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, tolerances, true, gPvd);
  PxInitExtensions(*gPhysics, gPvd);
  gDispatcher = PxDefaultCpuDispatcherCreate(0);
  PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
  sceneDesc.cpuDispatcher = gDispatcher;
  sceneDesc.gravity = PxVec3(0, -15.81f, 0);
  sceneDesc.filterShader = contactReportFilterShader;
  sceneDesc.simulationEventCallback = &gContactReportCallback;
  sceneDesc.solverType = PxSolverType::eTGS;
  gScene = gPhysics->createScene(sceneDesc);

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
  PX_RELEASE(gScene);
  PX_RELEASE(gDispatcher);
  PxCloseExtensions();
  PX_RELEASE(gPhysics);
  if (gPvd)
  {
    PxPvdTransport* transport = gPvd->getTransport();
    gPvd->release();
    gPvd = NULL;
    PX_RELEASE(transport);
  }
  PX_RELEASE(gFoundation);
}

void Physics::PhysicsManager::Simulate(float dt)
{
#if FIXED_STEP
  static float accumulator = 0;
  static const float step = 1.0f / 60.0f;
  accumulator += dt;
  static bool resultsReady = true;
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
#endif
  if (!resultsReady)
    return;
  // update all entity transforms whose actor counterpart was updated
  const auto actorTypes = PxActorTypeFlag::eRIGID_DYNAMIC | PxActorTypeFlag::eRIGID_STATIC;
  const auto numActors = gScene->getNbActors(actorTypes);
  if (numActors > 0)
  {
    std::vector<PxRigidDynamic*> actors(numActors);
    gScene->getActors(actorTypes, reinterpret_cast<PxActor**>(actors.data()), numActors);
    for (auto actor : actors)
    {
      const bool sleeping = actor->is<PxRigidDynamic>() ? actor->is<PxRigidDynamic>()->isSleeping() : false;
      if (sleeping)
        continue;
      const auto& pose = actor->getGlobalPose();
      Entity entity = gActors[actor];
      if (entity)
      {
        auto& tr = entity.GetComponent<Components::Transform>();
        tr.SetTranslation(*(glm::vec3*) & pose.p);
        glm::quat q(*(glm::quat*) & pose.q);
        tr.SetRotation(glm::toMat4(q));
      }
    }
  }
}

physx::PxRigidDynamic* Physics::PhysicsManager::AddDynamicActorEntity(Entity entity, MaterialType material, BoxCollider collider)
{
  const auto& tr = entity.GetComponent<Components::Transform>();
  //collider.halfExtents *= tr.GetScale();
  glm::quat q(tr.GetRotation());
  glm::vec3 pos(tr.GetTranslation());
  PxTransform tr2(toPxVec3(pos), toPxQuat(q));
  PxBoxGeometry geom(toPxVec3(collider.halfExtents));
  PxRigidDynamic* dynamic = PxCreateDynamic(*gPhysics, tr2, geom, *gMaterials[(int)material], 10.0f);
  dynamic->setAngularDamping(0.5f);
  gActors[dynamic] = entity;
  gScene->addActor(*dynamic);
  return dynamic;
}

physx::PxRigidDynamic* Physics::PhysicsManager::AddDynamicActorEntity(Entity entity, MaterialType material, CapsuleCollider collider)
{
  const auto& tr = entity.GetComponent<Components::Transform>();
  glm::quat q(tr.GetRotation());
  glm::vec3 pos(tr.GetTranslation());
  PxTransform tr2(toPxVec3(pos), toPxQuat(q));
  PxCapsuleGeometry geom(collider.radius, collider.halfHeight);
  PxRigidDynamic* dynamic = PxCreateDynamic(*gPhysics, tr2, geom, *gMaterials[(int)material], 10.0f);
  dynamic->setAngularDamping(0.5f);
  gActors[dynamic] = entity;
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
  gActors[pStatic] = entity;
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
  gActors[pStatic] = entity;
  gScene->addActor(*pStatic);
  return pStatic;
}

void Physics::PhysicsManager::RemoveActorEntity(physx::PxRigidActor* actor)
{
  if (actor)
  {
    gScene->removeActor(*actor);
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
