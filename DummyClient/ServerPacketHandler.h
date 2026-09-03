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
	PKT_C_REGISTER = 1002,
	PKT_S_REGISTER = 1003,
	PKT_C_ROOM_LIST = 1004,
	PKT_S_ROOM_LIST = 1005,
	PKT_C_CREATE_ROOM = 1006,
	PKT_S_CREATE_ROOM = 1007,
	PKT_C_ENTER_ROOM = 1008,
	PKT_S_ENTER_ROOM = 1009,
	PKT_C_LEAVE_ROOM = 1010,
	PKT_S_LEAVE_ROOM = 1011,
	PKT_C_CHANGE_PLAYER_TYPE = 1012,
	PKT_C_CHANGE_TEAM = 1013,
	PKT_C_READY = 1014,
	PKT_S_ROOM_STATE = 1015,
	PKT_C_START_MATCH = 1016,
	PKT_S_MATCH_PREPARE = 1017,
	PKT_C_MATCH_PREPARE = 1018,
	PKT_S_MATCH_START = 1019,
	PKT_S_MATCH_STATE = 1020,
	PKT_S_PLAYER_STATE = 1021,
	PKT_S_PLAYER_RESPAWN = 1022,
	PKT_C_MOVE = 1023,
	PKT_S_MOVE = 1024,
	PKT_C_FIRE = 1025,
	PKT_S_FIRE = 1026,
	PKT_C_HIT = 1027,
	PKT_S_PLAYER_DESPAWN = 1028,
	PKT_S_MATCH_END = 1029,
	PKT_C_RETURN_TO_ROOM = 1030,
	PKT_S_RETURN_TO_ROOM = 1031,
	PKT_C_CHAT = 1032,
	PKT_S_CHAT = 1033,
};


// Custom Handlers
bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len);
bool Handle_S_LOGIN(PacketSessionRef& session, Protocol::S_LOGIN& pkt);
bool Handle_S_REGISTER(PacketSessionRef& session, Protocol::S_REGISTER& pkt);
bool Handle_S_ROOM_LIST(PacketSessionRef& session, Protocol::S_ROOM_LIST& pkt);
bool Handle_S_CREATE_ROOM(PacketSessionRef& session, Protocol::S_CREATE_ROOM& pkt);
bool Handle_S_ENTER_ROOM(PacketSessionRef& session, Protocol::S_ENTER_ROOM& pkt);
bool Handle_S_LEAVE_ROOM(PacketSessionRef& session, Protocol::S_LEAVE_ROOM& pkt);
bool Handle_S_ROOM_STATE(PacketSessionRef& session, Protocol::S_ROOM_STATE& pkt);
bool Handle_S_MATCH_PREPARE(PacketSessionRef& session, Protocol::S_MATCH_PREPARE& pkt);
bool Handle_S_MATCH_START(PacketSessionRef& session, Protocol::S_MATCH_START& pkt);
bool Handle_S_MATCH_STATE(PacketSessionRef& session, Protocol::S_MATCH_STATE& pkt);
bool Handle_S_PLAYER_STATE(PacketSessionRef& session, Protocol::S_PLAYER_STATE& pkt);
bool Handle_S_PLAYER_RESPAWN(PacketSessionRef& session, Protocol::S_PLAYER_RESPAWN& pkt);
bool Handle_S_MOVE(PacketSessionRef& session, Protocol::S_MOVE& pkt);
bool Handle_S_FIRE(PacketSessionRef& session, Protocol::S_FIRE& pkt);
bool Handle_S_PLAYER_DESPAWN(PacketSessionRef& session, Protocol::S_PLAYER_DESPAWN& pkt);
bool Handle_S_MATCH_END(PacketSessionRef& session, Protocol::S_MATCH_END& pkt);
bool Handle_S_RETURN_TO_ROOM(PacketSessionRef& session, Protocol::S_RETURN_TO_ROOM& pkt);
bool Handle_S_CHAT(PacketSessionRef& session, Protocol::S_CHAT& pkt);


class ServerPacketHandler
{
public:
	static void Init()
	{
		for (int i = 0; i < UINT16_MAX; i++)
			GPacketHandler[i] = Handle_INVALID;
		GPacketHandler[PKT_S_LOGIN] = [](PacketSessionRef& session, BYTE* buffer, int32 len) {	return HandlePacket<Protocol::S_LOGIN>(Handle_S_LOGIN, session, buffer, len);	};
		GPacketHandler[PKT_S_REGISTER] = [](PacketSessionRef& session, BYTE* buffer, int32 len) {	return HandlePacket<Protocol::S_REGISTER>(Handle_S_REGISTER, session, buffer, len);	};
		GPacketHandler[PKT_S_ROOM_LIST] = [](PacketSessionRef& session, BYTE* buffer, int32 len) {	return HandlePacket<Protocol::S_ROOM_LIST>(Handle_S_ROOM_LIST, session, buffer, len);	};
		GPacketHandler[PKT_S_CREATE_ROOM] = [](PacketSessionRef& session, BYTE* buffer, int32 len) {	return HandlePacket<Protocol::S_CREATE_ROOM>(Handle_S_CREATE_ROOM, session, buffer, len);	};
		GPacketHandler[PKT_S_ENTER_ROOM] = [](PacketSessionRef& session, BYTE* buffer, int32 len) {	return HandlePacket<Protocol::S_ENTER_ROOM>(Handle_S_ENTER_ROOM, session, buffer, len);	};
		GPacketHandler[PKT_S_LEAVE_ROOM] = [](PacketSessionRef& session, BYTE* buffer, int32 len) {	return HandlePacket<Protocol::S_LEAVE_ROOM>(Handle_S_LEAVE_ROOM, session, buffer, len);	};
		GPacketHandler[PKT_S_ROOM_STATE] = [](PacketSessionRef& session, BYTE* buffer, int32 len) {	return HandlePacket<Protocol::S_ROOM_STATE>(Handle_S_ROOM_STATE, session, buffer, len);	};
		GPacketHandler[PKT_S_MATCH_PREPARE] = [](PacketSessionRef& session, BYTE* buffer, int32 len) {	return HandlePacket<Protocol::S_MATCH_PREPARE>(Handle_S_MATCH_PREPARE, session, buffer, len);	};
		GPacketHandler[PKT_S_MATCH_START] = [](PacketSessionRef& session, BYTE* buffer, int32 len) {	return HandlePacket<Protocol::S_MATCH_START>(Handle_S_MATCH_START, session, buffer, len);	};
		GPacketHandler[PKT_S_MATCH_STATE] = [](PacketSessionRef& session, BYTE* buffer, int32 len) {	return HandlePacket<Protocol::S_MATCH_STATE>(Handle_S_MATCH_STATE, session, buffer, len);	};
		GPacketHandler[PKT_S_PLAYER_STATE] = [](PacketSessionRef& session, BYTE* buffer, int32 len) {	return HandlePacket<Protocol::S_PLAYER_STATE>(Handle_S_PLAYER_STATE, session, buffer, len);	};
		GPacketHandler[PKT_S_PLAYER_RESPAWN] = [](PacketSessionRef& session, BYTE* buffer, int32 len) {	return HandlePacket<Protocol::S_PLAYER_RESPAWN>(Handle_S_PLAYER_RESPAWN, session, buffer, len);	};
		GPacketHandler[PKT_S_MOVE] = [](PacketSessionRef& session, BYTE* buffer, int32 len) {	return HandlePacket<Protocol::S_MOVE>(Handle_S_MOVE, session, buffer, len);	};
		GPacketHandler[PKT_S_FIRE] = [](PacketSessionRef& session, BYTE* buffer, int32 len) {	return HandlePacket<Protocol::S_FIRE>(Handle_S_FIRE, session, buffer, len);	};
		GPacketHandler[PKT_S_PLAYER_DESPAWN] = [](PacketSessionRef& session, BYTE* buffer, int32 len) {	return HandlePacket<Protocol::S_PLAYER_DESPAWN>(Handle_S_PLAYER_DESPAWN, session, buffer, len);	};
		GPacketHandler[PKT_S_MATCH_END] = [](PacketSessionRef& session, BYTE* buffer, int32 len) {	return HandlePacket<Protocol::S_MATCH_END>(Handle_S_MATCH_END, session, buffer, len);	};
		GPacketHandler[PKT_S_RETURN_TO_ROOM] = [](PacketSessionRef& session, BYTE* buffer, int32 len) {	return HandlePacket<Protocol::S_RETURN_TO_ROOM>(Handle_S_RETURN_TO_ROOM, session, buffer, len);	};
		GPacketHandler[PKT_S_CHAT] = [](PacketSessionRef& session, BYTE* buffer, int32 len) {	return HandlePacket<Protocol::S_CHAT>(Handle_S_CHAT, session, buffer, len);	};		
	}

	static bool HandlePacket(PacketSessionRef& session, BYTE* buffer, int32 len)
	{
		PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
		return GPacketHandler[header->id](session, buffer, len);
	}
	static SendBufferRef MakeSendBuffer(Protocol::C_LOGIN& pkt) { return MakeSendBuffer(pkt, PKT_C_LOGIN); }
	static SendBufferRef MakeSendBuffer(Protocol::C_REGISTER& pkt) { return MakeSendBuffer(pkt, PKT_C_REGISTER); }
	static SendBufferRef MakeSendBuffer(Protocol::C_ROOM_LIST& pkt) { return MakeSendBuffer(pkt, PKT_C_ROOM_LIST); }
	static SendBufferRef MakeSendBuffer(Protocol::C_CREATE_ROOM& pkt) { return MakeSendBuffer(pkt, PKT_C_CREATE_ROOM); }
	static SendBufferRef MakeSendBuffer(Protocol::C_ENTER_ROOM& pkt) { return MakeSendBuffer(pkt, PKT_C_ENTER_ROOM); }
	static SendBufferRef MakeSendBuffer(Protocol::C_LEAVE_ROOM& pkt) { return MakeSendBuffer(pkt, PKT_C_LEAVE_ROOM); }
	static SendBufferRef MakeSendBuffer(Protocol::C_CHANGE_PLAYER_TYPE& pkt) { return MakeSendBuffer(pkt, PKT_C_CHANGE_PLAYER_TYPE); }
	static SendBufferRef MakeSendBuffer(Protocol::C_CHANGE_TEAM& pkt) { return MakeSendBuffer(pkt, PKT_C_CHANGE_TEAM); }
	static SendBufferRef MakeSendBuffer(Protocol::C_READY& pkt) { return MakeSendBuffer(pkt, PKT_C_READY); }
	static SendBufferRef MakeSendBuffer(Protocol::C_START_MATCH& pkt) { return MakeSendBuffer(pkt, PKT_C_START_MATCH); }
	static SendBufferRef MakeSendBuffer(Protocol::C_MATCH_PREPARE& pkt) { return MakeSendBuffer(pkt, PKT_C_MATCH_PREPARE); }
	static SendBufferRef MakeSendBuffer(Protocol::C_MOVE& pkt) { return MakeSendBuffer(pkt, PKT_C_MOVE); }
	static SendBufferRef MakeSendBuffer(Protocol::C_FIRE& pkt) { return MakeSendBuffer(pkt, PKT_C_FIRE); }
	static SendBufferRef MakeSendBuffer(Protocol::C_HIT& pkt) { return MakeSendBuffer(pkt, PKT_C_HIT); }
	static SendBufferRef MakeSendBuffer(Protocol::C_RETURN_TO_ROOM& pkt) { return MakeSendBuffer(pkt, PKT_C_RETURN_TO_ROOM); }
	static SendBufferRef MakeSendBuffer(Protocol::C_CHAT& pkt) { return MakeSendBuffer(pkt, PKT_C_CHAT); }

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