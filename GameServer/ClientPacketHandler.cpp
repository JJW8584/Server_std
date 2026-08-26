#include "pch.h"
#include "ClientPacketHandler.h"
#include "BufferReader.h"
#include "BufferWriter.h"
#include "ObjectUtils.h"
#include "Room.h"
#include "Player.h"

PacketHandlerFunc GPacketHandler[UINT16_MAX];

// ÄÁÅÙÃ÷ ÀÛ¾÷
bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len)
{
	return false;
}

bool Handle_C_LOGIN(PacketSessionRef& session, Protocol::C_LOGIN& pkt)
{
	Protocol::S_LOGIN loginPkt;
	loginPkt.set_success(true);

	auto gameSession = static_pointer_cast<GameSession>(session);
	UINT64 userId;

	PlayerRef existingPlayer = gameSession->player.load();

	if (existingPlayer != nullptr)
	{
		userId = existingPlayer->GetObjectId();
	}
	else
	{
		PlayerRef userRef = ObjectUtils::CreatePlayer(gameSession);
		userId = userRef->GetObjectId();
		loginPkt.set_object_id(userId);
	}

	SEND_PACKET(loginPkt);

	cout << "User " << userId << " Login" << endl;

	return true;
}

bool Handle_C_ROOM_LIST(PacketSessionRef& session, Protocol::C_ROOM_LIST& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

	GRoomManager->DoAsync(&RoomManager::HandleRoomList, gameSession);

	return true;
}

bool Handle_C_CREATE_ROOM(PacketSessionRef& session, Protocol::C_CREATE_ROOM& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

	GRoomManager->DoAsync(&RoomManager::HandleCreateRoom, gameSession, pkt);

	return true;
}

bool Handle_C_ENTER_ROOM(PacketSessionRef& session, Protocol::C_ENTER_ROOM& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);
	
	GRoomManager->DoAsync(&RoomManager::HandleEnterRoom, gameSession, pkt);

	return true;
}

bool Handle_C_LEAVE_ROOM(PacketSessionRef& session, Protocol::C_LEAVE_ROOM& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

	PlayerRef player = gameSession->player.load();
	if (player == nullptr)
	{
		Protocol::S_LEAVE_ROOM leaveRoomPkt;
		leaveRoomPkt.set_success(false);
		SEND_PACKET(leaveRoomPkt);

		return true;
	}

	RoomRef room = player->room.load().lock();
	if (room == nullptr)
	{
		Protocol::S_LEAVE_ROOM leaveRoomPkt;
		leaveRoomPkt.set_success(false);
		SEND_PACKET(leaveRoomPkt);

		return true;
	}

	room->DoAsync(&Room::HandleLeavePlayer, player);

	return true;
}

bool Handle_C_CHANGE_PLAYER_TYPE(PacketSessionRef& session, Protocol::C_CHANGE_PLAYER_TYPE& pkt)
{
	return true;
}

bool Handle_C_CHANGE_TEAM(PacketSessionRef& session, Protocol::C_CHANGE_TEAM& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

	PlayerRef player = gameSession->player.load();
	if (player == nullptr)
		return false;

	RoomRef room = player->room.load().lock();
	if (room == nullptr)
		return false;

	room->DoAsync(&Room::HandleChangeTeam, player, pkt);
	return true;
}

bool Handle_C_READY(PacketSessionRef& session, Protocol::C_READY& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

	PlayerRef player = gameSession->player.load();
	if (player == nullptr)
		return false;

	RoomRef room = player->room.load().lock();
	if (room == nullptr)
		return false;

	room->DoAsync(&Room::HandleReadyState, player, pkt.ready());

	cout << "ready" << endl;

	return true;
}

bool Handle_C_START_MATCH(PacketSessionRef& session, Protocol::C_START_MATCH& pkt)
{
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

	PlayerRef player = gameSession->player.load();
	if (player == nullptr)
		return false;

	RoomRef room = player->room.load().lock();
	if (room == nullptr)
		return false;

	room->DoAsync(&Room::HandleStartMatch, player);

	return true;
}

bool Handle_C_MATCH_PREPARE(PacketSessionRef& session, Protocol::C_MATCH_PREPARE& pkt)
{
	return true;
}

bool Handle_C_MOVE(PacketSessionRef& session, Protocol::C_MOVE& pkt)
{
	auto gameSession = static_pointer_cast<GameSession>(session);

	PlayerRef player = gameSession->player.load();
	if (player == nullptr)
		return false;

	RoomRef room = player->room.load().lock();
	if (room == nullptr)
		return false;

	// TODO : À¯Àú Ã¼Å©

	//room->HandleMove(pkt);

	room->DoAsync(&Room::HandleMove, pkt, player->GetObjectId());

	return true;
}

bool Handle_C_FIRE(PacketSessionRef& session, Protocol::C_FIRE& pkt)
{
	return true;
}

bool Handle_C_CHAT(PacketSessionRef& session, Protocol::C_CHAT& pkt)
{
	cout << pkt.msg() << endl;

	return true;
}
