#pragma once
#include "NetEvent.h"
#include <NetDefines.h>

namespace Net
{
	// a packet that is ready to be sent and interpreted over a network
	struct Packet
	{
		// constructing a packet
		Packet(int type, void* data, size_t data_size)
		{
			bufSize_ = data_size + PACKET_HEADER_LEN;
			buf = new std::byte[bufSize_];
			reinterpret_cast<int*>(buf)[0] = type;
			std::memcpy(buf + PACKET_HEADER_LEN, data, bufSize_ - PACKET_HEADER_LEN);
		}

		// interpreting an enet packet
		// TODO: make this constructor it's own class that doesn't require copying data
		Packet(const ENetPacket* ev)
		{
			bufSize_ = ev->dataLength;
			buf = new std::byte[ev->dataLength];
			std::memcpy(buf, ev->data, ev->dataLength);
		}

		~Packet()
		{
			delete[] buf;
		}

		int GetType() const { return reinterpret_cast<int*>(buf)[0]; }
		std::byte* GetData() { return buf + PACKET_HEADER_LEN; }
		std::byte* GetBuffer() { return buf; }
		const std::byte* GetBuffer() const { return buf; }
		const size_t GetSize() const { return bufSize_; }

	private:
		size_t bufSize_;
		std::byte* buf;
	};

	// read-only view of a packet- doesn't manage, can't modify
	struct PacketView
	{
		PacketView(const Packet& packet) : bufptr(packet.GetBuffer()) {}
		PacketView(const ENetPacket* packet) : bufptr(reinterpret_cast<std::byte*>(packet->data)) {}

		int GetType() const { return reinterpret_cast<const int*>(bufptr)[0]; }
		const std::byte* GetData() const { return bufptr + PACKET_HEADER_LEN; }
		const std::byte* GetBuffer() const { return bufptr; }

	private:
		const std::byte* bufptr;
	};

	// concurrent structure for asynchronously processing outgoing network events
	struct EventController
	{
	public:
		void PushEvent(Packet&& p)
		{
			std::lock_guard lk(mtx);
			backBuffer.push(std::move(p));
		}

		Packet PopEvent()
		{
			std::lock_guard lk(mtx);
			Packet&& t = std::move(packetBuf.front());
			packetBuf.pop();
			return t;
		}

		void SwapBuffers()
		{
			std::lock_guard lk(mtx);
			packetBuf.swap(backBuffer);
		}

	private:
		std::queue<Packet> packetBuf;
		std::queue<Packet> backBuffer;
		std::shared_mutex mtx;
	};
}