#pragma once
#include <unordered_map>
#include <memory>
#include <thread>
#include <chrono>
#include <shared_mutex>
#include <optional>
#include "Timer.h"

struct PhysicsObject
{
	virtual void Update() = 0;
	virtual std::unique_ptr<PhysicsObject> Clone() = 0;

	enum ObjectTypes { tPlayerPhysics, };

	int type = -1;
};


struct PlayerPhysics final : public PhysicsObject
{
	PlayerPhysics() { PhysicsObject::type = PhysicsObject::tPlayerPhysics; }
	void Update() override {};
	std::unique_ptr<PhysicsObject> Clone() override
	{
		return std::make_unique<PlayerPhysics>(*this);
	}

	glm::vec3 pos{ 0, 0, 0 };
	glm::vec3 rot{ 0, 0, 0 };
};


// fixed-tick player physics simulation
class PhysicsWorld
{
public:
	// spawns a thread running the physics simulation
	static void Init();
	static void Shutdown();
	static void PushObject(int id, std::unique_ptr<PhysicsObject> obj);
	static void PopObject(int id);
	static std::unique_ptr<PhysicsObject> GetObject(int id);

private:
	static void run(); // run in another thread
	static void cleanup();

	static inline std::unordered_map<int, std::unique_ptr<PhysicsObject>> objects;
	static inline std::unique_ptr<std::thread> thread;
	static inline bool shutdownThreads = false;
	static inline std::shared_mutex mtx;
};