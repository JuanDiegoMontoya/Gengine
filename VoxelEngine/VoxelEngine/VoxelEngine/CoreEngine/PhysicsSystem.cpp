#include "PhysicsSystem.h"
#include "Components.h"
#include <entt/src/core/algorithm.hpp>

#define _DEBUG
#include <PxPhysicsAPI.h>
#include <foundation/PxAllocatorCallback.h>
#include <foundation/PxErrorCallback.h>

#define PX_RELEASE(x)	if(x)	{ x->release(); x = NULL;	}
#define PVD_DEBUG 0

//class AllocatorCallback : public physx::PxAllocatorCallback
//{
//public:
//	virtual void* allocate(size_t size, const char* typeName, const char* filename, int line) override
//	{
//		return (void*)_aligned_malloc(size, 16);
//	}
//
//	virtual void deallocate(void* ptr)
//	{
//		_aligned_free(ptr);
//	}
//};
//
//class UserErrorCallback : public physx::PxErrorCallback
//{
//public:
//	virtual void reportError(physx::PxErrorCode::Enum code, const char* message, const char* file,
//		int line)
//	{
//		switch (code)
//		{
//		case physx::PxErrorCode::eNO_ERROR:
//			break;
//		case physx::PxErrorCode::eDEBUG_INFO:
//			break;
//		case physx::PxErrorCode::eDEBUG_WARNING:
//			break;
//		case physx::PxErrorCode::ePERF_WARNING:
//			break;
//		case physx::PxErrorCode::eABORT:
//			break;
//		case physx::PxErrorCode::eOUT_OF_MEMORY: [[fallthrough]];
//		case physx::PxErrorCode::eINTERNAL_ERROR: [[fallthrough]];
//		case physx::PxErrorCode::eINVALID_PARAMETER: [[fallthrough]];
//		case physx::PxErrorCode::eINVALID_OPERATION: [[fallthrough]];
//		case physx::PxErrorCode::eMASK_ALL: [[fallthrough]];
//		default:
//			ASSERT_MSG(false, "Something bad happened in PhysX");
//			break;
//		}
//		// error processing implementation
//		//...
//	}
//};

using namespace physx;
namespace
{
	PxFilterFlags contactReportFilterShader(PxFilterObjectAttributes attributes0, PxFilterData filterData0,
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

	ContactReportCallback gContactReportCallback;

	PxDefaultAllocator		gAllocator;
	PxDefaultErrorCallback	gErrorCallback;

	PxFoundation* gFoundation = NULL;
	PxPhysics* gPhysics = NULL;

	PxDefaultCpuDispatcher* gDispatcher = NULL;
	PxScene* gScene = NULL;
	PxMaterial* gMaterial = NULL;
	PxPvd* gPvd = NULL;
}

PhysicsSystem::PhysicsSystem()
{
	gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);
#if PVD_DEBUG
	gPvd = PxCreatePvd(*gFoundation);
	PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
	gPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);
#endif
	gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(), true, gPvd);
	PxInitExtensions(*gPhysics, gPvd);
	gDispatcher = PxDefaultCpuDispatcherCreate(0);
	PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
	sceneDesc.cpuDispatcher = gDispatcher;
	sceneDesc.gravity = PxVec3(0, -9.81f, 0);
	sceneDesc.filterShader = contactReportFilterShader;
	sceneDesc.simulationEventCallback = &gContactReportCallback;
	gScene = gPhysics->createScene(sceneDesc);

	PxPvdSceneClient* pvdClient = gScene->getScenePvdClient();
	if (pvdClient)
	{
		pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
	}
	gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.6f);

	PxRigidStatic* groundPlane = PxCreatePlane(*gPhysics, PxPlane(0, 1, 0, 0), *gMaterial);
	gScene->addActor(*groundPlane);
}

PhysicsSystem::~PhysicsSystem()
{
	PX_RELEASE(gScene);
	PX_RELEASE(gDispatcher);
	PxCloseExtensions();
	PX_RELEASE(gPhysics);
	if (gPvd)
	{
		PxPvdTransport* transport = gPvd->getTransport();
		gPvd->release();	gPvd = NULL;
		PX_RELEASE(transport);
	}
	PX_RELEASE(gFoundation);
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
			physics.velocity += physics.acceleration * dt;
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

	//ASSERT(scene.GetRegistry().sortable<Components::Transform>());
	//scene.GetRegistry().sort<Components::Transform>(
	//	[&scene](const entt::entity lhs, const entt::entity rhs)
	//	{
	//		return Entity(lhs, &scene).GetHierarchyHeight() > Entity(rhs, &scene).GetHierarchyHeight();
	//	}, entt::insertion_sort());

	// update local transforms
	{
		using namespace Components;
		// create a PARTIALLY OWNING group, OWNING TRANSFORM
		auto group = scene.GetRegistry().group<Transform>(entt::get<LocalTransform, Parent>);
		group.sort(
			[&scene](const entt::entity lhs, const entt::entity rhs)
		{
			return Entity(lhs, &scene).GetHierarchyHeight() > Entity(rhs, &scene).GetHierarchyHeight();
		}, entt::insertion_sort());

		for (auto entity : group)
		{
			auto [worldTransform, localTransform, parent] = group.get<Transform, LocalTransform, Parent>(entity);
			auto& ltransform = localTransform.transform;
			bool localDirty = ltransform.IsDirty();
			if (ltransform.IsDirty())
			{
				auto model = glm::mat4(1);
				model *= glm::translate(glm::mat4(1), ltransform.GetTranslation());
				model *= ltransform.GetRotation();
				model *= glm::scale(glm::mat4(1), ltransform.GetScale());
				ltransform.SetModel(model);
			}

			const auto& parentTransform = scene.GetRegistry().get<Components::Transform>(parent.entity);
			if (parentTransform.IsDirty() || localDirty)
			{
				worldTransform.SetTranslation(parentTransform.GetTranslation() + ltransform.GetTranslation() * parentTransform.GetScale());

				worldTransform.SetScale(ltransform.GetScale() * parentTransform.GetScale());

				worldTransform.SetTranslation(worldTransform.GetTranslation() - parentTransform.GetTranslation());
				worldTransform.SetTranslation(glm::mat3(parentTransform.GetRotation()) * worldTransform.GetTranslation());
				worldTransform.SetTranslation(worldTransform.GetTranslation() + parentTransform.GetTranslation());

				worldTransform.SetRotation(ltransform.GetRotation() * parentTransform.GetRotation());
			}
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
