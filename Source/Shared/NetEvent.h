#pragma once
#include <queue>
#include <shared_mutex>
#include <enet/enet.h>
#include <NetDefines.h>

#define REGISTER_EVENT(x) x* x ## _data
//#define PROCESS_EVENT(x) case Packet::e ## x: data.x ## _data->Process(); break
#define UNDEFINED_EVENT(x) ;

namespace Net
{
	//struct ClientJoinEvent
	//{
	//};

	enum ClientEvent
	{
		/* Client Join Event
			Sent to the server upon initial connection.
			The server will return a result stating whether the
			connection was accepted or declined. This is different
			from simply connecting to the server's socket, as
			this event will give the server discretionary control
			over what clients may or may not join.
		*/
		eClientJoinEvent,

		/* Client Leave Event
			Sent to server before client disconnects. No acknowledgement
			required. This is purely for the server's convenience.
			The server will disconnect the client forcibly if their timeout
			limit is reached.
		*/
		eClientLeaveEvent,

		/* Client Print vec3 Event
			Test event. When sent, the server will print the value
			of the vec3 contained within.
		*/
		eClientPrintVec3Event,

		/* Client Input
			Sent after polling inputs each client tick.
			Aggregate of all input actions a client can take in a frame.
			Booleans packed into bytes indicating whether an action is taken.
		*/
		eClientInput,
	};

	enum ServerEvent
	{
		/* Server Join Result Event
			Sent to a particular client after they sent a ClientJoinEvent.
			The server will inform the client if their connection request
			was successful and, if it was, what their assigned client ID is.
		*/
		eServerJoinResultEvent,

		/* Server List Players
			Broadcast to every client when a player joins or disconnects from
			the server.
		*/
		eServerListPlayersEvent,

		/* Server Game State
			Broadcast to all clients each server tick.
			Contains dynamical visible information, such as player and mob
			positions, or player-held items.
		*/
		eServerGameState,
	};

	struct ClientPrintVec3Event
	{
		glm::vec3 v;
	};

	struct ServerJoinResultEvent
	{
		bool success;
		int id; // uninitialized if success is false
	};

	struct ServerListPlayersEvent
	{
		int connected;
		int* IDs;
	};

	struct ServerGameState
	{
		struct PlayerState
		{
			int id;
			glm::vec3 pos{ 0, 0, 0 };
			glm::vec3 front{ 1, 0, 0 };
		};

		ServerGameState(std::vector<PlayerState>& data)
		{
			buf = new std::byte[sizeof(int) + data.size() * sizeof(PlayerState)];
			reinterpret_cast<int*>(buf)[0] = data.size();
			std::memcpy(buf + sizeof(int), data.data(), data.size() * sizeof(PlayerState));
		}

		//ServerGameState(const Packet& packet)
		//{
		//	buf = new std::byte[packet.]
		//}

		ServerGameState(const std::byte* mem)
		{
			int size = *reinterpret_cast<const int*>(mem);
			buf = new std::byte[sizeof(int) + size * sizeof(PlayerState)];
			std::memcpy(buf, mem, sizeof(int) + size * sizeof(PlayerState));
		}

		ServerGameState(const ENetPacket* packet)
		{
			buf = new std::byte[packet->dataLength - PACKET_HEADER_LEN];
			std::memcpy(buf, packet->data, packet->dataLength - PACKET_HEADER_LEN);
		}

		~ServerGameState() { delete[] buf; }

		int GetNumPlayers() const { return reinterpret_cast<int*>(buf)[0]; }
		PlayerState* GetPlayerStates()
		{
			return reinterpret_cast<PlayerState*>(buf + sizeof(int));
		}

		size_t GetSize() const
		{
			return sizeof(int) + GetNumPlayers() * sizeof(PlayerState);
		}

		std::byte* GetBuffer() { return buf; }

	private:
		std::byte* buf;
	};

	struct ClientInput
	{
		// TODO: pack the inputs, perhaps into a bitfield?
		bool moveFoward;
		bool moveBack;
		bool moveLeft;
		bool moveRight;
		bool jump;
	};
}





#if 0
// below is some weird experimental garbage that I made for fun :)
// DON'T ACTUALLY USE THIS CLASS!
// TODO: add bits to this
template<class ... Names>
struct BoolField
{
	template<unsigned I>
	struct Proxy
	{
		static_assert(I < 8, "BoolField too large!");

		Proxy(uint8_t& data) : data_(data) {}
		Proxy(const Proxy& other) : data_(other.data_) {}
		
		operator bool() { return data_ & (1 << I); };
		Proxy<I>& operator=(bool rhs) { data_ |= rhs << I; }

	private:
		uint8_t& data_;
	};

	template<class Name>
	auto Get() // TODO: remove
	{
		return Proxy<getIndex(Name)>(data_);
	}

private:
	consteval int getIndex(auto name)
	{
		int i = 0;
		for (const auto N : { Names... })
		{
			if constexpr (name == N)
				return i;
			++i;
		}

		static_assert(0, "Failed to find member name in template args!");
	}

	uint8_t data_;
};
#endif