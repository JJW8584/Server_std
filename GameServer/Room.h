#pragma once
#include "pch.h"
#include "JobQueue.h"

class Room : public JobQueue
{
public:
	Room(GameSessionRef session, uint64 roomId,	string roomName, uint32 maxPlayerCount);
	virtual ~Room();

	uint64 GetRoomId();
	Protocol::RoomInfo GetRoomInfo();

	bool HandleEnterPlayer(PlayerRef player);
	bool HandleLeavePlayer(PlayerRef player);
	bool HandleChangeTeam(PlayerRef player, Protocol::C_CHANGE_TEAM pkt);
	bool HandleReadyState(PlayerRef player, bool ready);
	bool HandleStartMatch(PlayerRef player);
	void HandleMove(Protocol::C_MOVE pkt, const uint64 objectId);
	void HandleFire(PlayerRef player, Protocol::C_FIRE pkt);
	void HandleHit(PlayerRef player, Protocol::C_HIT pkt);
	void HandlePrepareMatch(PlayerRef player);
	void PlayerDespawn(uint64 objectId);
	void PlayerRespawn(uint64 objectId);
	void HandleReturnRoom(GameSessionRef session);

public:
	//0.1초마다 1번씩 실행
	void UpdateTick();

	RoomRef GetRoomRef();

private:
	bool EnterPlayer(PlayerRef player);
	bool LeavePlayer(uint64 objectId);

private:
	void Broadcast(SendBufferRef sendBuffer, uint64 exceptId = 0);

private:
	bool _isClosing = false;
	unordered_map<uint64, PlayerRef> _players;

	USE_LOCK;
	Protocol::RoomInfo _roomInfo;

	Protocol::MatchInfo _matchInfo;
	Protocol::MatchResult _matchResult;

	uint32 _sendRemainSecondsTimer = 10;
	uint32 _remainSeconds;
};

class RoomManager : public JobQueue
{
public:
	void HandleRoomList(GameSessionRef session);
	void HandleCreateRoom(GameSessionRef session, Protocol::C_CREATE_ROOM pkt);
	void HandleEnterRoom(GameSessionRef session, Protocol::C_ENTER_ROOM pkt);

	void RemoveRoom(uint64 roomId);

private:
	uint64 CreateRoomId();

private:
	atomic<uint64> _roomIdGenerator = 1;
	unordered_map<uint64, RoomRef> _rooms;
};

extern RoomManagerRef GRoomManager;