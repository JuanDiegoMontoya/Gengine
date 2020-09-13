#include "WorldGen2.h"
#include <Chunks/Chunk.h>
#include <Chunks/ChunkStorage.h>
#include <Chunks/ChunkHelpers.h>
#include <execution>
#include <FastNoiseSIMD/FastNoiseSIMD.h>
#include <Utilities/Timer.h>

namespace WorldGen2
{
	namespace
	{
#if 0
		glm::ivec3 lowChunkDim{ 0, 0, 0 };
		glm::ivec3 highChunkDim{ 70, 10, 70 };
#else
		glm::ivec3 lowChunkDim{ 0, 0, 0 };
		glm::ivec3 highChunkDim{ 5, 3, 5 };
#endif
	}

	// init chunks that we finna modify
	void Init()
	{
		Timer timer;
		for (int x = lowChunkDim.x; x < highChunkDim.x; x++)
		{
			//printf("\nX: %d", x);
			for (int y = lowChunkDim.y; y < highChunkDim.y; y++)
			{
				//printf(" Y: %d", y);
				for (int z = lowChunkDim.z; z < highChunkDim.z; z++)
				{
					Chunk* newChunk = new Chunk({ x, y, z });
					ChunkStorage::GetMapRaw()[{ x, y, z }] = newChunk;
				}
			}
		}
		printf("Allocating chunks took %f seconds\n", timer.elapsed());
	}

	// Parameters: (density > .05) == solid
	//   lacunarity = 2, octaves = 5
	// Value noise: good for structured, geometric looking locations
	//   generates consistently sized caverns with good connectivity
	// Fractal perlin: grand, lumpy, almost chiseled-like terrain
	//   big rocky areas, good connectivity

	// does the thing
	void GenerateWorld()
	{
		Timer timer;
		FastNoiseSIMD* noisey = FastNoiseSIMD::NewFastNoiseSIMD();
		noisey->SetFractalLacunarity(2.0);
		noisey->SetFractalOctaves(5);
		//noisey->SetFrequency(.04);
		//noisey->SetPerturbType(FastNoiseSIMD::Gradient);
		//noisey->SetPerturbAmp(0.4);
		//noisey->SetPerturbFrequency(0.4);
		
		auto& chunks = ChunkStorage::GetMapRaw();
		std::for_each(std::execution::par, chunks.begin(), chunks.end(),
			[&](auto pair)
		{
			if (pair.second)
			{
				glm::ivec3 st = pair.first * Chunk::CHUNK_SIZE;
				float* noiseSet = noisey->GetCubicFractalSet(st.z, st.y, st.x, 
					Chunk::CHUNK_SIZE, Chunk::CHUNK_SIZE, Chunk::CHUNK_SIZE, 1);
				int idx = 0;

				//printf(".");
				glm::ivec3 pos, wpos;
				for (pos.z = 0; pos.z < Chunk::CHUNK_SIZE; pos.z++)
				{
					//int zcsq = pos.z * Chunk::CHUNK_SIZE_SQRED;
					for (pos.y = 0; pos.y < Chunk::CHUNK_SIZE; pos.y++)
					{
						//int yczcsq = pos.y * Chunk::CHUNK_SIZE + zcsq;
						for (pos.x = 0; pos.x < Chunk::CHUNK_SIZE; pos.x++)
						{
							//int index = pos.x + yczcsq;
							wpos = ChunkHelpers::chunkPosToWorldPos(pos, pair.first);

							//double density = noise.GetValue(wpos.x, wpos.y, wpos.z); // chunks are different
							//double density = noise.GetValue(pos.x, pos.y, pos.z); // same chunk every time
							//density = 0;

							//if (pos.z < 8 && pos.x < 8 && pos.y < 8)
							//	ChunkStorage::SetBlockType(wpos, BlockType::bStone);
							//continue;

							float density = noiseSet[idx++];
							if (density < -.02)
							{
								ChunkStorage::SetBlockType(wpos, BlockType::bGrass);
							}
							if (density < -.03)
							{
								ChunkStorage::SetBlockType(wpos, BlockType::bDirt);
							}
							if (density < -.04)
							{
								ChunkStorage::SetBlockType(wpos, BlockType::bStone);
							}

							//printf("%f\n", density);
						}
					}
				}

				FastNoiseSIMD::FreeNoiseSet(noiseSet);
			}
			else
			{
				printf("null chunk doe\n");
			}
		});

		delete noisey;
		printf("Generating chunks took %f seconds\n", timer.elapsed());
	}


	void InitSunlight()
	{
		
	}


	void InitMeshes()
	{
		Timer timer;
		auto& chunks = ChunkStorage::GetMapRaw();
		std::for_each(std::execution::par,
			chunks.begin(), chunks.end(), [](auto& p)
		{
			if (p.second)
				p.second->BuildMesh();
		});
		printf("Generating meshes took %f seconds\n", timer.elapsed());
	}


	void InitBuffers()
	{
		Timer timer;
		auto& chunks = ChunkStorage::GetMapRaw();
		std::for_each(std::execution::seq,
			chunks.begin(), chunks.end(), [](auto& p)
		{
			if (p.second)
				p.second->BuildBuffers();
		});
		printf("Buffering meshes took %f seconds\n", timer.elapsed());
	}
}