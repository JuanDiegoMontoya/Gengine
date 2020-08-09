#include "stdafx.h"
#define NOMINMAX
#include <iostream>

#include <zlib.h>
#include <cassert>

#include <Packet.h>
#include "Server.h"
#include <Timer.h>
#include <NetDefines.h>

void testCompress()
{
	std::string compressThis = { R"100(
Now, this is a story all about how
My life got flipped-turned upside down
And I'd like to take a minute
Just sit right there
I'll tell you how I became the prince of a town called Bel Air
In west Philadelphia born and raised
On the playground was where I spent most of my days
Chillin' out maxin' relaxin' all cool
And all shootin some b-ball outside of the school
When a couple of guys who were up to no good
Started making trouble in my neighborhood
I got in one little fight and my mom got scared
She said 'You're movin' with your auntie and uncle in Bel Air'
I begged and pleaded with her day after day
But she packed my suit case and sent me on my way
She gave me a kiss and then she gave me my ticket.
I put my Walkman on and said, 'I might as well kick it'.
First class, yo this is bad
Drinking orange juice out of a champagne glass.
Is this what the people of Bel-Air living like?
Hmm this might be alright.
But wait I hear they're prissy, bourgeois, all that
Is this the type of place that they just send this cool cat?
I don't think so
I'll see when I get there
I hope they're prepared for the prince of Bel-Air
Well, the plane landed and when I came out
There was a dude who looked like a cop standing there with my name out
I ain't trying to get arrested yet
I just got here
I sprang with the quickness like lightning, disappeared
I whistled for a cab and when it came near
The license plate said fresh and it had dice in the mirror
If anything I could say that this cab was rare
But I thought 'Nah, forget it' - 'Yo, homes to Bel Air'
I pulled up to the house about seven or eigth
And I yelled to the cabbie 'Yo homes smell ya later'
I looked at my kingdom
I was finally there
To sit on my throne as the Prince of Bel Air)100" };
	ULONG sizeDataUncompressed = compressThis.size() * sizeof(char);
	ULONG sizeDataCompressed = sizeDataUncompressed * 1.1 + 12;
	BYTE* dataCompressed = new BYTE[sizeDataCompressed];
	int z_result = compress(
		dataCompressed,
		&sizeDataCompressed,
		(BYTE*)compressThis.c_str(),
		compressThis.size()
	);
	if (z_result != Z_OK)
		printf("FAILURE!!!!!!!!!!!!!!!!!!!!!\n");
	printf("Uncompressed size: %d\n", sizeDataUncompressed);
	printf("Compressed size: %d\n", sizeDataCompressed);
	printf("%s", compressThis.c_str());
	for (int i = 0; i < sizeDataCompressed; i++)
		putchar(dataCompressed[i]);
	printf("\n--\n\n");

	printf("Uncompressing data...\n");
	BYTE* dataUncompressed = new BYTE[sizeDataUncompressed];
	z_result = uncompress(
		dataUncompressed,
		&sizeDataUncompressed,
		dataCompressed,
		sizeDataCompressed
	);
	if (z_result != Z_OK)
		printf("FAILURE!!!!!!!!!!!!!!!!!!!!!\n");
	printf("Uncompressed, size = %d\n", sizeDataUncompressed);
	for (int i = 0; i < sizeDataUncompressed; i++)
		putchar(dataUncompressed[i]);

	assert(strcmp((char*)dataUncompressed, compressThis.c_str()) == 0);
	delete[] dataCompressed;
	delete[] dataUncompressed;
}

namespace Net
{
	bool Server::Init()
	{
		// a. Initialize enet
		if (enet_initialize() != 0)
		{
			printf("An error occured while initializing ENet.\n");
			return false;
		}

		// b. Create a host using enet_host_create
		address.host = ENET_HOST_ANY;
		address.port = 1234;
		server = enet_host_create(&address, 32, 2, 0, 0);

		if (server == NULL)
		{
			printf("An error occured while trying to create an ENet server host\n");
			return false;
		}

		// c. Connect and user service
		eventStatus = 1;

		printf("Server started successfully!\n");

		thread = std::make_unique<std::thread>([this]() { this->run(); });
		return true;
	}


	void Server::run()
	{
		Timer timer;
		double accum = 0;
		while (!shutdownThreads)
		{
			accum += timer.elapsed();
			timer.reset();

			eventStatus = enet_host_service(server, &event, 1000);

			// If we had some event that interested us
			if (eventStatus > 0)
			{
				switch (event.type)
				{
				case ENET_EVENT_TYPE_CONNECT:
				{
					printf("(Server) We got a new connection from %x\n",
						event.peer->address.host);
					char *buf = new char[1000];
					sprintf(buf, "%X, %X", event.peer->address.host, event.peer->address.port);
					event.peer->data = buf;
					break;
				}
				case ENET_EVENT_TYPE_RECEIVE:
				{
					//printf("(Server) Message from client : %s\n", event.packet->data);
					//// Lets broadcast this message to all
					//enet_host_broadcast(server, 0, event.packet);

					// we are guaranteed to receive at least the type identifier
					assert(event.packet->dataLength >= sizeof(int));
					//printf("Packet from: %s\n", event.peer->data);
					
					Net::Packet packet(event.packet);
					ProcessClientEvent(packet, event.peer);
					break;
				}
				case ENET_EVENT_TYPE_DISCONNECT:
				{
					printf("%s disconnected.\n", event.peer->data);

					// Reset client's information
					if (clients.erase(event.peer->address))
					{
						connectedPlayers--;
						broadcastPlayerList();
					}
					delete[] event.peer->data;
					event.peer->data = NULL;
					break;
				}
				}
			}

			while (accum > SERVER_NET_TICK)
			{
				accum -= SERVER_NET_TICK;

				// perform a server tick action here

				// generate server gamestate
				{
					std::vector<ServerGameState::PlayerState> playerStates;
					for (const auto& [addr, client] : clients)
					{
						ServerGameState::PlayerState state;
						state.id = client.clientID;
						state.pos = client.playerData.pos;
						state.front = client.playerData.front;
						playerStates.push_back(state);
					}

					ServerGameState ev(playerStates);
					Packet pk(Net::eServerGameState, ev.GetBuffer(), ev.GetSize());
					ENetPacket* packet = enet_packet_create(pk.GetBuffer(), pk.GetSize(), ENET_PACKET_FLAG_RELIABLE);

					////auto data = ServerGameState(packet);
					////auto* data = reinterpret_cast<ServerGameState*>(pk.GetData());
					//auto data = ServerGameState(pk.GetData());
					////auto* data = &ev;
					//auto* states = data.GetPlayerStates();
					//for (int i = 0; i < data.GetNumPlayers(); i++)
					//{
					//	printf("%d", states[i].id);
					//}

					enet_host_broadcast(server, 0, packet);
				}
			}

		}
	}


	void Server::Shutdown()
	{
		shutdownThreads = true;
		thread->join();
	}


	void Server::cleanup()
	{
		enet_deinitialize();
	}


	void Server::ProcessClientEvent(Packet& packet, ENetPeer* peer)
	{
		switch (packet.GetType())
		{
		case Net::eClientJoinEvent:
		{
			processJoinEvent(peer);
			break;
		}
		case Net::eClientLeaveEvent:
		{
			printf("A client disconnected!\n");
			break;
		}
		case Net::eClientPrintVec3Event:
		{
			glm::vec3 v = reinterpret_cast<ClientPrintVec3Event*>(packet.GetData())->v;
			clients[peer->address].playerData.pos = v;
			//printf("(%f, %f, %f)\n", v.x, v.y, v.z);
			break;
		}
		case Net::eClientInput:
		{
			auto in = reinterpret_cast<ClientInput*>(packet.GetData());
			// TODO: update client's physics state based on input
			//printf("Client input: ");
			//std::cout << in->jump << in->moveBack << in->moveFoward << in->moveLeft << in->moveRight << std::endl;
			break;
		}
		default:
		{
			//assert(false && "The type hasn't been defined yet!");
			printf("Unsupported data type sent from a client! (%d)\n", packet.GetType());
		}
		} // end switch
	}


	void Server::processJoinEvent(ENetPeer* peer)
	{
		ServerJoinResultEvent event{ true };

		// connection failed (player count reached)
		if (connectedPlayers >= maxPlayers)
			event.success = false;
		else
		{
			connectedPlayers++;
			auto& client = clients[peer->address];
			client.clientID = nextClientID++;
			event.id = client.clientID;
		}

		// tell the client whether their connection attempt succeeded
		Packet packet(Net::eServerJoinResultEvent, &event, sizeof(event));
		ENetPacket* resultPacket = enet_packet_create(packet.GetBuffer(), sizeof(int) + sizeof(event), ENET_PACKET_FLAG_RELIABLE);
		enet_peer_send(peer, 0, resultPacket);

		if (!event.success)
		{
			printf("Client failed to connect\n");
		}
		else
		{
			// tell the client that their connection succeeded, then broadcast their existence to the server
			printf("A client connected!\n");
			broadcastPlayerList();
		}
	}


	void Server::broadcastPlayerList()
	{			
		// put all client IDs into temporary contiguous memory for memcpy later
		std::vector<int> IDs;
		for (const auto& p : clients)
			IDs.push_back(p.second.clientID);

		// TODO: change this jank way of constructing a packet
		size_t bufsize = 2 + clients.size(); // packet type, num clients, data
		int* buf = new int[bufsize];
		buf[0] = Net::eServerListPlayersEvent;
		buf[1] = IDs.size();
		std::memcpy(buf + 2, IDs.data(), IDs.size() * sizeof(int));

		ENetPacket* packet = enet_packet_create(buf, bufsize * sizeof(int), ENET_PACKET_FLAG_RELIABLE);
		enet_host_broadcast(server, 0, packet);
		delete[] buf;
	}
}