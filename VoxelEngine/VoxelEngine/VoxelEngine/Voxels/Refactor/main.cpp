#if 0
#include "stdafx.h"
#include <Engine.h>
#include "Interface.h"
//#include "World.h"
#include <Rendering/Renderer.h>
#include <Rendering/NuRenderer.h>

#include "Client.h"

//#include <Utilities/BitArray.h>
//void TestBitArray()
//{
//	BitArray coom(50);
//	coom.SetSequence(5, 8, 0b11001101);
//	std::bitset<8> bb(coom.GetSequence(5, 8));
//	std::cout << bb << std::endl;
//}

//#include <Graphics/BufferAllocator.h>
//void TestAllocator()
//{
//	ChunkVBOAllocator allocator(25001, 8);
//	std::cout << std::boolalpha << allocator.Allocate((Chunk*)1, 0, 1) << std::endl;
//	for (int i = 2; i < 51; i++)
//		std::cout << std::boolalpha << allocator.Allocate((Chunk*)i, 0, rand() % 500) << std::endl;
//	std::cout << std::boolalpha << allocator.Allocate((Chunk*)69, 0, 25100) << std::endl;
//	//std::cout << std::boolalpha << allocator.Free((Chunk*)21) << std::endl;
//	for (int i = 0; i < 50; i++)
//		std::cout << std::boolalpha << allocator.Free((Chunk*)i) << std::endl;
//	std::cout << std::boolalpha << allocator.FreeOldest() << std::endl;
//	allocator.Free((Chunk*)1);
//	allocator.Allocate((Chunk*)1, 0, 50);
//	allocator.Free((Chunk*)1);
//	allocator.Allocate((Chunk*)1, 0, 50);
//}

int main()
{
	EngineConfig cfg;
	cfg.verticalSync = false;
	Engine::Init(cfg);

	Renderer::Init();
	NuRenderer::Init();
	Interface::Init();
	World::Init();

	Engine::Run();

	World::Shutdown();
	Engine::Cleanup();

	return 0;
}
#endif