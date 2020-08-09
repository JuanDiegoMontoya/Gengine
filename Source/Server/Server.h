#pragma once
#include <enet/enet.h>
#include <thread>
#include <memory>
#include "ClientInfo.h"
#include <unordered_map>

struct Packet;

namespace std
{
	template<>
	struct hash<ENetAddress>
	{
		size_t operator()(const ENetAddress& v) const
		{
			return hash<enet_uint32>()(v.host) ^ hash<enet_uint16>()(v.port);
		}
	};

	inline bool operator==(const ENetAddress& a, const ENetAddress& b)
	{
		return a.host == b.host && a.port == b.port;
	}

}

namespace Net
{
	class Server
	{
	public:
		// launches thread that processes network events in the background
		bool Init();
		void Shutdown();

	private:
		void run();
		void cleanup();
		void ProcessClientEvent(Packet& packet, ENetPeer* peer);

		void processJoinEvent(ENetPeer* peer);
		void broadcastPlayerList();

		int maxPlayers = 10;
		int connectedPlayers = 0;
		int nextClientID = 0;
		std::unordered_map<ENetAddress, ClientInfo> clients;

		bool shutdownThreads = false;
		std::unique_ptr<std::thread> thread;

		ENetAddress address;
		ENetHost* server;
		ENetEvent event;
		int eventStatus;
	};
}