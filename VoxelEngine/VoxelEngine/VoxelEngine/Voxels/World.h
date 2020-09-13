#pragma once
#include "chunk_manager.h"
#include "GameObject.h"
//#include "sun.h"
#include "hud.h"

#include "Client.h"

class Sun;
class HUD;

// contains a description of everything in the world
class World
{
public:
	static void Init();
	static void Shutdown();
	static void Update();
	static void CheckCollision();
	static void CheckCollision2();
	static void CheckInteraction();

	// unconditionally updates a block at a position
	//void UpdateBlockAt(glm::ivec3 wpos, Block bl);
	//void GenerateBlockAt(glm::ivec3 wpos, Block b); // updates a block at a position IF it isn't written yet
	//void GenerateBlockAtCheap(glm::ivec3 wpos, Block b);


	//private
	static void checkBlockPlacement();
	static void checkBlockDestruction();
	static void checkBlockPick();


	// "private" variables

	static inline ChunkManager chunkManager_;
	static inline HUD hud_;

	static inline std::vector<GameObject*> objects_; // all game objects in the scene
	static inline glm::vec3 bgColor_ = glm::vec3(.529f, .808f, .922f); // sky blue

	static inline Sun* sun_;
	static inline bool doCollisionTick = true;
	// debug
	static inline int debugCascadeQuad = 0;

	static inline Net::Client client;
};