#include "stdafx.h"
#include "FixedPhysics.h"
#include <NetDefines.h>


void PhysicsWorld::PushObject(int id, std::unique_ptr<PhysicsObject> obj)
{
	std::lock_guard lk(mtx);
	objects[id] = std::move(obj);
}


void PhysicsWorld::PopObject(int id)
{
	std::lock_guard lk(mtx);
	objects.erase(id);
}

// TODO: return std::optional for consistency...
// can't use PhysicsObject&& in std::optional...
// look at polymorphic_value for this
std::unique_ptr<PhysicsObject> PhysicsWorld::GetObject(int id)
{
	if (auto f = objects.find(id); f != objects.end())
		return f->second->Clone();
	return nullptr;
}


void PhysicsWorld::Init()
{
	thread = std::make_unique<std::thread>(run);
}


void PhysicsWorld::Shutdown()
{
	shutdownThreads = true;
	thread->join();
	PhysicsWorld::cleanup();
}


void PhysicsWorld::run()
{
	std::shared_mutex mtx;
	Timer timer;
	double accum = 0;
	// accumulate time
	while (!shutdownThreads)
	{
		accum += timer.elapsed();
		timer.reset();

		while (accum >= SERVER_PHYSICS_TICK)
		{
			accum -= SERVER_PHYSICS_TICK;

			for (auto& [id, obj] : objects)
			{
				obj->Update();
			}
		}
	}
}


void PhysicsWorld::cleanup()
{

}
