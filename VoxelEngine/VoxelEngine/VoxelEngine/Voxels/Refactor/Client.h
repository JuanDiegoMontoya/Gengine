#pragma once

#include <Packet.h>
#include <enet/enet.h>
#include "NetPlayerState.h"

struct Packet;

namespace Net
{
	class Client
	{
	public:
		// runs in thread, updating in the background
		void Init();
		void Shutdown();
		void DisconnectFromCurrent();
		void Connect(std::string addr, int port);

		void RenderPlayers();

		PlayerWorld& GetPlayerWorld() { return playerWorld; }
		int GetThisID() { return thisID; }
		bool GetConnected() { return connected; }

	private:
		void MainLoop();

		Net::ClientInput GetActions();
		void ProcessServerEvent(Packet& packet);

		void processJoinResultEvent(Packet& packet);
		void processServerListPlayersEvent(Packet& packet);
		void processServerGameState(Packet& packet);

		Net::EventController eventQueue;
		std::unique_ptr<std::thread> thread;

		bool shutdownThreads = false;
		bool connected = false;

		ENetAddress address;
		ENetHost* client;
		ENetPeer* peer;
		ENetEvent event;
		int eventStatus;

		PlayerWorld playerWorld;
		int thisID = -1;
	};
}