#pragma once
#include "prefab.h"
#include <noise/noise.h>

typedef struct Chunk* ChunkPtr;
typedef class Level* LevelPtr;


enum class TerrainType : unsigned
{
	tNone,  // unfilled terrain (replaced with ocean or something)
	tPlains,
	tHills,
	tOcean,

	tCount
};


class WorldGen
{
public:

	/* 
		Generates a rectangular world of the given dimensions
		(in chunks). Sparsity of the blocks can be controlled.
		
		*Size: number of chunks to generate in that direction
		sparse (0-1): chance to generate a non-air block
		updateList: located within level calling this function
	*/
	static void GenerateSimpleWorld(int xSize, int ySize, int zSize, float sparse, std::vector<ChunkPtr>& updateList);
	
	static void GenerateHeightMapWorld(int x, int z);

	//static void GenerateChunkMap()

	/*
		Populates a chunk based on its position in the world
	*/
	static void InitNoiseFuncs();
	static void GenerateChunk(glm::ivec3 cpos);
	static TerrainType GetTerrainType(glm::ivec3 wpos);
	static double GetTemperature(double x, double y, double z);
	static double GetHumidity(double x, double z);

	static void GeneratePrefab(const Prefab& pfab, glm::ivec3 wpos);

	static void Generate3DNoiseChunk(glm::ivec3 cpos);

	// returns a density at a particular point
	static double GetDensity(const glm::vec3& wpos);
private:
	// sample near values in heightmap to obtain rough first derivative
	static float getSlope(noise::model::Plane& pl, int x, int z);

	// you can't make this object
	WorldGen() = delete;

	static int global_seed_;
};