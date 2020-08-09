#include "stdafx.h"
#include "FixedSizeWorld.h"
#include "ChunkStorage.h"
#include <execution>


void FixedSizeWorld::GenWorld(glm::ivec3 lowChunkDim, glm::ivec3 highChunkDim)
{
	for (int x = lowChunkDim.x; x < highChunkDim.x; x++)
	{
		printf("\nX: %d", x);
		for (int y = lowChunkDim.y; y < highChunkDim.y; y++)
		{
			printf(" Y: %d", y);
			for (int z = lowChunkDim.z; z < highChunkDim.z; z++)
			{
				Chunk* newChunk = new Chunk();
				newChunk->SetPos({ x, y, z });
				ChunkStorage::GetMapRaw()[{ x, y, z }] = newChunk;
				//WorldGen::GenerateChunk({ x, y, z });
			}
		}
	}

	auto& chunks = ChunkStorage::GetMapRaw();
	auto lambruh = [&]()
	{
		std::for_each(std::execution::par,
			chunks.begin(), chunks.end(), [](auto& p)
		{
			if (p.second)
				p.second->BuildMesh();
			else
				printf("null chunk?!?!?\n");
		});
	};
	lambruh();
	//std::thread coom = std::thread(lambruh);
}
