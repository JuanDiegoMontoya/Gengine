#include "stdafx.h"

#include "Server.h"
#include "FixedPhysics.h"
#include <iostream>

int main()
{
	// start the network thread
	Net::Server server;
	bool result = server.Init();
	if (result == false)
		return EXIT_FAILURE;

	PhysicsWorld::Init();
	PlayerPhysics playa;
	playa.pos = { -2, -2, -2 };
	playa.rot = { 0, 0, 0 };
	PhysicsWorld::PushObject(0, std::make_unique<PlayerPhysics>(playa));

	// begin an interactive session
	printf("Welcome to the interactive server console!\n");
	printf("Input a command.\n");
	while (1)
	{
		std::cout << ": ";
		std::string in;
		std::cin >> in;
		std::cout << "\nYou wrote: " << in;
		std::cout << std::endl;

		if (in == "q" || in == "quit" || in == "exit")
			break;
	}

	PhysicsWorld::Shutdown();
	server.Shutdown();
}