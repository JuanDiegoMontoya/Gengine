#include "stdafx.h"
#include "Client.h"
#include <NetDefines.h>

#include <Rendering/Renderer.h>
#include <Pipeline.h>
#include <Components/Camera.h>
#include <Packet.h>
#include <Systems/InputSystem.h>
#include <Timer.h>

#include <enet/enet.h>
#include <zlib.h>
#include <concurrent_queue.h>

#include <thread>
#include <sstream>

#include <Rendering/Renderer.h>
#include <Rendering/NuRenderer.h>
#include <Systems/Graphics/shader.h>
#include <Components/Camera.h>
#include <Pipeline.h>
#include <Engine.h>

#pragma optimize("", off)
namespace Net
{
	void Client::Init()
	{
		// a. Initialize enet
		if (enet_initialize() != 0)
		{
			printf("An error occured while initializing ENet.\n");
			exit(EXIT_FAILURE);
		}

		// b. Create a host using enet_host_create
		client = enet_host_create(NULL, 1, 2, 57600 / 8, 14400 / 8);

		if (client == NULL)
		{
			printf("An error occured while trying to create an ENet server host\n");
			exit(EXIT_FAILURE);
		}
	}


	void Client::Connect(std::string addr, int port)
	{
		if (thread)
			DisconnectFromCurrent();

		enet_address_set_host(&address, addr.c_str());
		address.port = port;

		// c. Connect and user service
		peer = enet_host_connect(client, &address, 2, 0);

		if (peer == NULL)
		{
			printf("No available peers for initializing an ENet connection");
			exit(EXIT_FAILURE);
		}

		eventStatus = 1;

		// not sure why, but enet_host_service needs to be called before sending this packet
		enet_host_service(client, &event, CLIENT_NET_TICK * 1000);
		int join = Net::eClientJoinEvent;
		ENetPacket* joinRequest = enet_packet_create(&join, sizeof(join), ENET_PACKET_FLAG_RELIABLE);
		enet_peer_send(peer, 0, joinRequest);

		thread = std::make_unique<std::thread>([this]() { MainLoop(); });

		connected = true;
	}


	void Client::RenderPlayers()
	{
		ShaderPtr sr = Shader::shaders["flat_color"];
		sr->Use();
		sr->setVec4("u_color", { 1, 0, 1, 1 });
		sr->setMat4("u_proj", Renderer::GetPipeline()->GetCamera(0)->GetProj());
		sr->setMat4("u_view", Renderer::GetPipeline()->GetCamera(0)->GetView());

		for (auto [id, obj] : playerWorld.GetObjects_Unsafe()) // intentional copy
		{
			if (id == thisID)
				continue;
			auto vis = obj.GetVisibleState();
			//glm::mat4 model = glm::lookAt(-vis.pos, -vis.pos + vis.front, { 0, 1, 0 });
			glm::mat4 model = glm::translate(glm::mat4(1), vis.pos);
			sr->setMat4("u_model", model);
			Renderer::DrawCube();
		}

		sr->setVec4("u_color", { 1, 1, 0, 1 });
		//glm::mat4 model = glm::translate(glm::mat4(1), glm::vec3(0));
		glm::mat4 model = glm::lookAt(glm::vec3(0), glm::vec3(0) + glm::vec3(1), { 0, 1, 0 });
		sr->setMat4("u_model", model);
		Renderer::DrawCube();
		playerWorld.UpdateStates(Engine::GetDT());
	}


#pragma optimize("", off)
	void Client::MainLoop()
	{
		Timer timer;
		double accum = 0;
		while (!shutdownThreads)
		{
			accum += timer.elapsed();
			timer.reset();

			eventStatus = enet_host_service(client, &event, CLIENT_NET_TICK * 1000);

			// If we had some event that interested us
			if (eventStatus > 0)
			{
				switch (event.type)
				{
				case ENET_EVENT_TYPE_CONNECT: // someone else connected to us (shouldn't happen)
				{
					printf("(Client) We got a new connection from %x\n",
						event.peer->address.host);
					break;
				}
				case ENET_EVENT_TYPE_RECEIVE:
				{
					//printf("(Client) Message from server : %s\n", event.packet->data);
					// Lets broadcast this message to all
					// enet_host_broadcast(client, 0, event.packet);
					//enet_packet_destroy(event.packet);
					//printf("Received a message from the server (%x)\n", event.peer->address.host);

					Net::Packet packet(event.packet);
					//printf("Received event from server of type: %d\n", packet.GetType());
					ProcessServerEvent(packet);
					break;
				}
				case ENET_EVENT_TYPE_DISCONNECT: // someone else disconnected from us (shouldn't happen)
				{
					printf("(Client) %s disconnected.\n", event.peer->data);

					// Reset client's information
					event.peer->data = NULL;
					break;
				}
				}
			}

			while (accum > CLIENT_NET_TICK)
			{
				accum -= CLIENT_NET_TICK;

				// send client player state
				{
					auto temp = Renderer::GetPipeline()->GetCamera(0)->GetPos();
					Packet p(Net::eClientPrintVec3Event, &temp, sizeof(ClientPrintVec3Event));
					ENetPacket* packet = enet_packet_create(p.GetBuffer(), sizeof(int) + sizeof(ClientPrintVec3Event), ENET_PACKET_FLAG_RELIABLE);
					enet_peer_send(peer, 0, packet);
				}

				// send client input
				{
					auto temp = GetActions();
					Packet p(Net::eClientInput, &temp, sizeof(ClientInput));
					ENetPacket* packet = enet_packet_create(p.GetBuffer(), sizeof(int) + sizeof(ClientInput), ENET_PACKET_FLAG_RELIABLE);
					enet_peer_send(peer, 0, packet);
				}
			}
		}

		//DisconnectFromCurrent();
	}


	void Client::Shutdown()
	{
		if (thread)
		{
			shutdownThreads = true;
			thread->join();
			thread = nullptr;
		}
		enet_deinitialize();
	}


	void Client::DisconnectFromCurrent()
	{
		connected = false;

		if (!thread)
			return;

		shutdownThreads = true;
		thread->join();
		thread = nullptr;

		enet_peer_disconnect(peer, 0);
		/* Allow up to 3 seconds for the disconnect to succeed
		 * and drop any packets received packets.
		 */
		while (enet_host_service(client, &event, 3000) > 0)
		{
			switch (event.type)
			{
			case ENET_EVENT_TYPE_RECEIVE:
				enet_packet_destroy(event.packet);
				break;
			case ENET_EVENT_TYPE_DISCONNECT:
				printf("Disconnection succeeded.");
				return;
			}
		}
		/* We've arrived here, so the disconnect attempt didn't */
		/* succeed yet.  Force the connection down.             */
		enet_peer_reset(peer);
	}


	Net::ClientInput Client::GetActions()
	{
		Net::ClientInput ret;
		ret.jump = Input::Keyboard().pressed[GLFW_KEY_SPACE];
		ret.moveBack = Input::Keyboard().down[GLFW_KEY_S];
		ret.moveFoward = Input::Keyboard().down[GLFW_KEY_W];
		ret.moveLeft = Input::Keyboard().down[GLFW_KEY_A];
		ret.moveRight = Input::Keyboard().down[GLFW_KEY_D];
		return ret;
	}


	void Client::ProcessServerEvent(Net::Packet& packet)
	{
		switch (packet.GetType())
		{
		case Net::eServerJoinResultEvent:
		{
			processJoinResultEvent(packet);
			break;
		}
		case Net::eServerListPlayersEvent:
		{
			processServerListPlayersEvent(packet);
			break;
		}
		case Net::eServerGameState:
		{
			//printf("WE GOT A GAMESTATE EVENT BOIII\n");
			processServerGameState(packet);
			break;
		}
		default:
		{
			//assert(false && "The type hasn't been defined yet!");
			printf("Unsupported data type sent from the server! (%d)\n", packet.GetType());
		}
		}
	}


	void Client::processJoinResultEvent(Packet& packet)
	{
		auto event = *reinterpret_cast<ServerJoinResultEvent*>(packet.GetData());
		
		printf("Join result: %d\n", event.success);

		if (event.success)
		{
			printf("Our ID: %d\n", event.id);
			thisID = event.id;
			playerWorld.PushState(thisID, VisiblePlayerState());
		}
		else
		{
			DisconnectFromCurrent();
		}
	}


	void Client::processServerListPlayersEvent(Packet& packet)
	{
		auto event = *reinterpret_cast<ServerListPlayersEvent*>(packet.GetData());
		event.IDs = reinterpret_cast<int*>(packet.GetData() + sizeof(int));

		// remove all players that no longer exist in the world, and add any that were just added
		constexpr int existsClient = 1 << 0;
		constexpr int existsServer = 1 << 1;
		std::unordered_map<int, uint8_t> exists;
		for (const auto& [id, unused] : playerWorld.GetObjects_Unsafe())
			exists[id] |= existsClient;
		for (int i = 0; i < event.connected; i++)
			exists[event.IDs[i]] |= existsServer;

		for (const auto& [id, ex] : exists)
		{
			// erase if exists for client, but not server
			if (ex & existsClient && !(ex & existsServer))
			{
				printf("Removed player ID %d\n", id);
				playerWorld.GetObjects_Unsafe().erase(id);
			}

			// add if doesn't exist for client, but does for server
			if (!(ex & existsClient) && ex & existsServer)
			{
				printf("Added player ID %d\n", id);
				playerWorld.GetObjects_Unsafe().emplace(id, PlayerObject());
			}
		}
	}

	void Client::processServerGameState(Packet& packet)
	{
		auto data = Net::ServerGameState(packet.GetData());
		auto* states = data.GetPlayerStates();
		for (int i = 0; i < data.GetNumPlayers(); i++)
		{
			// no point updating visible state of something we cannot see
			if (states[i].id == thisID)
				continue;
			playerWorld.PushState(states[i].id, { states[i].pos, states[i].front });
		}
	}
}