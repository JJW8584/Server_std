#pragma once

#if UE_BUILD_DEBUG + UE_BUILD_DEVELOPMENT + UE_BUILD_TEST + UE_BUILD_SHIPPING >= 1
#include "S1.h"
#endif

#include "Protocol.pb.h"
#include "Enum.pb.h"
#include "Struct.pb.h"

using PacketHandlerFunc = std::function<bool(PacketSessionRef&, BYTE*, int32)>;
extern PacketHandlerFunc GPacketHandler[UINT16_MAX];

enum : uint16
{
	PKT_C_LOGIN = 1000,
	PKT_S_LOGIN = 1001,
	PKT_C_ROOM_LIST = 1002,
	PKT_S_ROOM_LIST = 1003,
	PKT_C_CREATE_ROOM = 1004,
	PKT_S_CREATE_ROOM = 1005,
	PKT_C_ENTER_ROOM = 1006,
	PKT_S_ENTER_ROOM = 1007,
	PKT_C_LEAVE_ROOM = 1008,
	PKT_S_LEAVE_ROOM = 1009,
	PKT_C_CHANGE_PLAYER_TYPE = 1010,
	PKT_C_CHANGE_TEAM = 1011,
	PKT_C_READY = 1012,
	PKT_S_ROOM_STATE = 1013,
	PKT_C_START_MATCH = 1014,
	PKT_S_MATCH_PREPARE = 1015,
	PKT_C_MATCH_PREPARE = 1016,
	PKT_S_MATCH_START = 1017,
	PKT_S_MATCH_STATE = 1018,
	PKT_S_PLAYER_STATE = 1019,
	PKT_S_PLAYER_RESPAWN = 1020,
	PKT_C_MOVE = 1021,
	PKT_S_MOVE = 1022,
	PKT_C_FIRE = 1023,
	PKT_S_FIRE = 1024,
	PKT_S_PLAYER_DESPAWN = 1025,
	PKT_S_MATCH_END = 1026,
	PKT_S_RETURN_TO_ROOM = 1027,
	PKT_C_CHAT = 1028,
	PKT_S_CHAT = 1029,
};


// Custom Handlers
bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len);
bool Handle_C_LOGIN(PacketSessionRef& session, Protocol::C_LOGIN& pkt);
bool Handle_C_ROOM_LIST(PacketSessionRef& session, Protocol::C_ROOM_LIST& pkt);
bool Handle_C_CREATE_ROOM(PacketSessionRef& session, Protocol::C_CREATE_ROOM& pkt);
bool Handle_C_ENTER_ROOM(PacketSessionRef& session, Protocol::C_ENTER_ROOM& pkt);
bool Handle_C_LEAVE_ROOM(PacketSessionRef& session, Protocol::C_LEAVE_ROOM& pkt);
bool Handle_C_CHANGE_PLAYER_TYPE(PacketSessionRef& session, Protocol::C_CHANGE_PLAYER_TYPE& pkt);
bool Handle_C_CHANGE_TEAM(PacketSessionRef& session, Protocol::C_CHANGE_TEAM& pkt);
bool Handle_C_READY(PacketSessionRef& session, Protocol::C_READY& pkt);
bool Handle_C_START_MATCH(PacketSessionRef& session, Protocol::C_START_MATCH& pkt);
bool Handle_C_MATCH_PREPARE(PacketSessionRef& session, Protocol::C_MATCH_PREPARE& pkt);
bool Handle_C_MOVE(PacketSessionRef& session, Protocol::C_MOVE& pkt);
bool Handle_C_FIRE(PacketSessionRef& session, Protocol::C_FIRE& pkt);
bool Handle_C_CHAT(PacketSessionRef& session, Protocol::C_CHAT& pkt);


class ClientPacketHandler
{
public:
	static void Init()
	{
		for (int i = 0; i < UINT16_MAX; i++)
			GPacketHandler[i] = Handle_INVALID;
		GPacketHandler[PKT_C_LOGIN] = [](PacketSessionRef& session, BYTE* buffer, int32 len) {	return HandlePacket<Protocol::C_LOGIN>(Handle_C_LOGIN, session, buffer, len);	};
		GPacketHandler[PKT_C_ROOM_LIST] = [](PacketSessionRef& session, BYTE* buffer, int32 len) {	return HandlePacket<Protocol::C_ROOM_LIST>(Handle_C_ROOM_LIST, session, buffer, len);	};
		GPacketHandler[PKT_C_CREATE_ROOM] = [](PacketSessionRef& session, BYTE* buffer, int32 len) {	return HandlePacket<Protocol::C_CREATE_ROOM>(Handle_C_CREATE_ROOM, session, buffer, len);	};
		GPacketHandler[PKT_C_ENTER_ROOM] = [](PacketSessionRef& session, BYTE* buffer, int32 len) {	return HandlePacket<Protocol::C_ENTER_ROOM>(Handle_C_ENTER_ROOM, session, buffer, len);	};
		GPacketHandler[PKT_C_LEAVE_ROOM] = [](PacketSessionRef& session, BYTE* buffer, int32 len) {	return HandlePacket<Protocol::C_LEAVE_ROOM>(Handle_C_LEAVE_ROOM, session, buffer, len);	};
		GPacketHandler[PKT_C_CHANGE_PLAYER_TYPE] = [](PacketSessionRef& session, BYTE* buffer, int32 len) {	return HandlePacket<Protocol::C_CHANGE_PLAYER_TYPE>(Handle_C_CHANGE_PLAYER_TYPE, session, buffer, len);	};
		GPacketHandler[PKT_C_CHANGE_TEAM] = [](PacketSessionRef& session, BYTE* buffer, int32 len) {	return HandlePacket<Protocol::C_CHANGE_TEAM>(Handle_C_CHANGE_TEAM, session, buffer, len);	};
		GPacketHandler[PKT_C_READY] = [](PacketSessionRef& session, BYTE* buffer, int32 len) {	return HandlePacket<Protocol::C_READY>(Handle_C_READY, session, buffer, len);	};
		GPacketHandler[PKT_C_START_MATCH] = [](PacketSessionRef& session, BYTE* buffer, int32 len) {	return HandlePacket<Protocol::C_START_MATCH>(Handle_C_START_MATCH, session, buffer, len);	};
		GPacketHandler[PKT_C_MATCH_PREPARE] = [](PacketSessionRef& session, BYTE* buffer, int32 len) {	return HandlePacket<Protocol::C_MATCH_PREPARE>(Handle_C_MATCH_PREPARE, session, buffer, len);	};
		GPacketHandler[PKT_C_MOVE] = [](PacketSessionRef& session, BYTE* buffer, int32 len) {	return HandlePacket<Protocol::C_MOVE>(Handle_C_MOVE, session, buffer, len);	};
		GPacketHandler[PKT_C_FIRE] = [](PacketSessionRef& session, BYTE* buffer, int32 len) {	return HandlePacket<Protocol::C_FIRE>(Handle_C_FIRE, session, buffer, len);	};
		GPacketHandler[PKT_C_CHAT] = [](PacketSessionRef& session, BYTE* buffer, int32 len) {	return HandlePacket<Protocol::C_CHAT>(Handle_C_CHAT, session, buffer, len);	};		
	}

	static bool HandlePacket(PacketSessionRef& session, BYTE* buffer, int32 len)
	{
		PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
		return GPacketHandler[header->id](session, buffer, len);
	}
	static SendBufferRef MakeSendBuffer(Protocol::S_LOGIN& pkt) { return MakeSendBuffer(pkt, PKT_S_LOGIN); }
	static SendBufferRef MakeSendBuffer(Protocol::S_ROOM_LIST& pkt) { return MakeSendBuffer(pkt, PKT_S_ROOM_LIST); }
	static SendBufferRef MakeSendBuffer(Protocol::S_CREATE_ROOM& pkt) { return MakeSendBuffer(pkt, PKT_S_CREATE_ROOM); }
	static SendBufferRef MakeSendBuffer(Protocol::S_ENTER_ROOM& pkt) { return MakeSendBuffer(pkt, PKT_S_ENTER_ROOM); }
	static SendBufferRef MakeSendBuffer(Protocol::S_LEAVE_ROOM& pkt) { return MakeSendBuffer(pkt, PKT_S_LEAVE_ROOM); }
	static SendBufferRef MakeSendBuffer(Protocol::S_ROOM_STATE& pkt) { return MakeSendBuffer(pkt, PKT_S_ROOM_STATE); }
	static SendBufferRef MakeSendBuffer(Protocol::S_MATCH_PREPARE& pkt) { return MakeSendBuffer(pkt, PKT_S_MATCH_PREPARE); }
	static SendBufferRef MakeSendBuffer(Protocol::S_MATCH_START& pkt) { return MakeSendBuffer(pkt, PKT_S_MATCH_START); }
	static SendBufferRef MakeSendBuffer(Protocol::S_MATCH_STATE& pkt) { return MakeSendBuffer(pkt, PKT_S_MATCH_STATE); }
	static SendBufferRef MakeSendBuffer(Protocol::S_PLAYER_STATE& pkt) { return MakeSendBuffer(pkt, PKT_S_PLAYER_STATE); }
	static SendBufferRef MakeSendBuffer(Protocol::S_PLAYER_RESPAWN& pkt) { return MakeSendBuffer(pkt, PKT_S_PLAYER_RESPAWN); }
	static SendBufferRef MakeSendBuffer(Protocol::S_MOVE& pkt) { return MakeSendBuffer(pkt, PKT_S_MOVE); }
	static SendBufferRef MakeSendBuffer(Protocol::S_FIRE& pkt) { return MakeSendBuffer(pkt, PKT_S_FIRE); }
	static SendBufferRef MakeSendBuffer(Protocol::S_PLAYER_DESPAWN& pkt) { return MakeSendBuffer(pkt, PKT_S_PLAYER_DESPAWN); }
	static SendBufferRef MakeSendBuffer(Protocol::S_MATCH_END& pkt) { return MakeSendBuffer(pkt, PKT_S_MATCH_END); }
	static SendBufferRef MakeSendBuffer(Protocol::S_RETURN_TO_ROOM& pkt) { return MakeSendBuffer(pkt, PKT_S_RETURN_TO_ROOM); }
	static SendBufferRef MakeSendBuffer(Protocol::S_CHAT& pkt) { return MakeSendBuffer(pkt, PKT_S_CHAT); }

private:
	template<typename PacketType, typename ProcessFunc>
	static bool HandlePacket(ProcessFunc func, PacketSessionRef& session, BYTE* buffer, int32 len)
	{
		PacketType pkt;
		if (pkt.ParseFromArray(buffer + sizeof(PacketHeader), len - sizeof(PacketHeader)) == false)
			return false;

		return func(session, pkt);
	}

	template<typename T>
	static SendBufferRef MakeSendBuffer(T& pkt, uint16 pktId)
	{
		const uint16 dataSize = static_cast<uint16>(pkt.ByteSizeLong());
		const uint16 packetSize = dataSize + sizeof(PacketHeader);

#if UE_BUILD_DEBUG + UE_BUILD_DEVELOPMENT + UE_BUILD_TEST + UE_BUILD_SHIPPING >= 1
		SendBufferRef sendBuffer = MakeShared<SendBuffer>(packetSize);
#else
		SendBufferRef sendBuffer = GSendBufferManager->Open(packetSize);
#endif

		PacketHeader* header = reinterpret_cast<PacketHeader*>(sendBuffer->Buffer());
		header->size = packetSize;
		header->id = pktId;

		pkt.SerializeToArray(&header[1], dataSize);

		sendBuffer->Close(packetSize);

		return sendBuffer;
	}
};