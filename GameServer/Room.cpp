#include "pch.h"
#include "Room.h"
#include "Player.h"

RoomManagerRef GRoomManager = MakeShared<RoomManager>();

Room::Room(GameSessionRef session, uint64 roomId, string roomName, uint32 maxPlayerCount)
{
	uint64 playerId = session->player.load()->GetObjectId();
	_roomInfo.set_host_object_id(playerId);
	_roomInfo.set_room_id(roomId);
	_roomInfo.set_room_name(roomName);
	_roomInfo.set_max_player_count(maxPlayerCount);
	_roomInfo.set_state(Protocol::ROOM_STATE_WAITING);
}

Room::~Room()
{
}

uint64 Room::GetRoomId()
{
	READ_LOCK;
	return _roomInfo.room_id();
}

Protocol::RoomInfo Room::GetRoomInfo()
{
	READ_LOCK;
	return _roomInfo;
}

bool Room::HandleEnterPlayer(PlayerRef player)
{
	if (player == nullptr)
		return false;

	Protocol::RoomInfo snapshot;
	{
		WRITE_LOCK;
		if (!EnterPlayer(player))
			return false;

		Protocol::RoomPlayerInfo* info = _roomInfo.add_players();

		info->set_object_id(player->GetObjectId());
		info->set_nickname(player->GetNickname());
		info->set_player_type(Protocol::PLAYER_TYPE_NONE);
		info->set_team(Protocol::TEAM_NONE);
		player->team = Protocol::TEAM_NONE;
		info->set_ready(false);

		snapshot.CopyFrom(_roomInfo);
	}

	Protocol::S_ROOM_STATE roomStatePkt;
	roomStatePkt.mutable_room_info()->CopyFrom(snapshot);
	SendBufferRef roomStateBuffer =	ClientPacketHandler::MakeSendBuffer(roomStatePkt);

	Broadcast(roomStateBuffer, player->GetObjectId());

	return true;
}


bool Room::HandleLeavePlayer(PlayerRef player)
{
	if (player == nullptr)
		return false;

	const uint64 objectId = player->GetObjectId();

	bool success = LeavePlayer(objectId);


	if (GameSessionRef session = player->session.lock())
	{
		Protocol::S_LEAVE_ROOM leaveRoomPkt;
		leaveRoomPkt.set_success(success);
		SEND_PACKET(leaveRoomPkt);
	}

	if (!success)
		return false;

	// 방이 비었을 때
	if (_players.empty())
	{
		_isClosing = true;

		const uint64 roomId = GetRoomId();

		GRoomManager->DoAsync(&RoomManager::RemoveRoom, roomId);

		return true;
	}

	// 방에 플레이어가 남아있을 때
	Protocol::S_ROOM_STATE roomStatePkt;
	{
		READ_LOCK;
		roomStatePkt.mutable_room_info()->CopyFrom(_roomInfo);
	}
	SendBufferRef roomStateBuffer = ClientPacketHandler::MakeSendBuffer(roomStatePkt);

	Broadcast(roomStateBuffer);

	return success;
}

bool Room::HandleChangeTeam(PlayerRef player, Protocol::C_CHANGE_TEAM pkt)
{
	if (player == nullptr)
		return false;

	const uint64 playerId = player->GetObjectId();

	if (_players.find(playerId) == _players.end())
		return false;

	Protocol::RoomInfo snapshot;
	{
		WRITE_LOCK;
		if (_roomInfo.state() != Protocol::ROOM_STATE_WAITING)
			return false;

		Protocol::RoomPlayerInfo* targetPlayerInfo = nullptr;
		for (int32 i = 0; i < _roomInfo.players_size(); ++i)
		{
			Protocol::RoomPlayerInfo* playerInfo = _roomInfo.mutable_players(i);

			if (playerInfo->object_id() == playerId)
			{
				targetPlayerInfo = playerInfo;
				break;
			}
		}

		if (targetPlayerInfo == nullptr)
			return false;

		if (pkt.team() != Protocol::TEAM_BLUE && pkt.team() != Protocol::TEAM_RED && pkt.team() != Protocol::TEAM_NONE)
			return false;

		// 이미 같은 팀이면 성공 처리만 하고 종료
		if (targetPlayerInfo->team() == pkt.team())
		{
			player->team = pkt.team();
			return true;
		}

		targetPlayerInfo->set_team(pkt.team());
		targetPlayerInfo->set_ready(false);
		player->team = pkt.team();

		snapshot.CopyFrom(_roomInfo);
	}

	Protocol::S_ROOM_STATE roomStatePkt;
	roomStatePkt.mutable_room_info()->CopyFrom(snapshot);

	Broadcast(ClientPacketHandler::MakeSendBuffer(roomStatePkt));

	return true;
}

bool Room::HandleReadyState(PlayerRef player, bool ready)
{
	if (player == nullptr)
		return false;

	uint64 playerId = player->GetObjectId();

	if (_players.find(playerId) == _players.end())
		return false;

	int32 playerIndex = -1;

	Protocol::RoomInfo snapshot;
	{
		WRITE_LOCK;
		for (int i = 0; i < _roomInfo.players_size(); ++i)
		{
			if (_roomInfo.players(i).object_id() == playerId)
			{
				playerIndex = i;
				break;
			}
		}

		if (playerIndex == -1)
			return false;

		if (_roomInfo.players(playerIndex).team() == Protocol::TEAM_NONE)
			return false;

		_roomInfo.mutable_players(playerIndex)->set_ready(ready);
		snapshot.CopyFrom(_roomInfo);
	}

	Protocol::S_ROOM_STATE roomStatePkt;
	roomStatePkt.mutable_room_info()->CopyFrom(snapshot);
	SendBufferRef roomStateBuffer = ClientPacketHandler::MakeSendBuffer(roomStatePkt);

	Broadcast(roomStateBuffer);

	return true;
}

bool Room::HandleStartMatch(PlayerRef player)
{
	if (player == nullptr)
		return false;

	const uint64 playerId = player->GetObjectId();

	if (_players.find(playerId) == _players.end())
	{
		//TODO: 실패 패킷 보내기
		return false;
	}

	WRITE_LOCK;
	if (playerId != _roomInfo.host_object_id())
	{
		//TODO: 실패 패킷 보내기
		return false;
	}

	for (int i = 0; i < _roomInfo.players_size(); ++i)
	{
		if (_roomInfo.players(i).object_id() == player->GetObjectId())
			continue;

		if (_roomInfo.players(i).ready() == false)
		{
			//TODO: 실패 패킷 보내기
			return false;
		}
	}
	Protocol::MatchInfo matchInfo;

	matchInfo.set_match_id(_roomInfo.room_id());
	matchInfo.set_duration_seconds(60);
	matchInfo.set_red_score(0);
	matchInfo.set_blue_score(0);
	for (int i = 0; i < _roomInfo.players_size(); ++i)
	{
		Protocol::RoomPlayerInfo roomPlayerInfo = _roomInfo.players(i);
		Protocol::MatchPlayerInfo matchPlayerInfo;
		Protocol::PlayerInfo playerInfo;
		Protocol::MoveInfo moveInfo;
		moveInfo.set_state(Protocol::MOVE_STATE_IDLE);
		switch (roomPlayerInfo.team())
		{
		case Protocol::TEAM_BLUE:
			moveInfo.set_x(-1300.f + 100.f * i);
			moveInfo.set_y(-1300.f + 100.f * i);
			moveInfo.set_z(100.f);
			break;
		case Protocol::TEAM_RED:
			moveInfo.set_x(1300.f + 100.f * i);
			moveInfo.set_y(1300.f + 100.f * i);
			moveInfo.set_z(100.f);
			break;
		}
		moveInfo.set_yaw(0);

		playerInfo.set_object_id(roomPlayerInfo.object_id());
		playerInfo.set_nickname(roomPlayerInfo.nickname());
		playerInfo.mutable_move_info()->CopyFrom(moveInfo);

		matchPlayerInfo.mutable_player_info()->CopyFrom(playerInfo);
		matchPlayerInfo.set_player_type(Protocol::PLAYER_TYPE_NONE);
		matchPlayerInfo.set_team(roomPlayerInfo.team());

		matchInfo.add_match_players_info()->CopyFrom(matchPlayerInfo);

		Protocol::MatchPlayerState matchPlayerState;
		matchPlayerState.set_hp(100);
		matchPlayerState.set_max_hp(100);
		matchPlayerState.set_is_alive(true);
		matchPlayerState.set_kill_count(0);
		matchPlayerState.set_death_count(0);

		matchInfo.add_match_players_state()->CopyFrom(matchPlayerState);

		_players[roomPlayerInfo.object_id()]->matchPlayerState.CopyFrom(matchPlayerState);

		_matchInfo.CopyFrom(matchInfo);
	}
	
	_roomInfo.set_state(Protocol::ROOM_STATE_LOADING);

	Protocol::S_MATCH_PREPARE matchPreparePkt;
	matchPreparePkt.mutable_match_info()->CopyFrom(matchInfo);
	SendBufferRef SendBuffer = ClientPacketHandler::MakeSendBuffer(matchPreparePkt);

	Broadcast(SendBuffer);

	return true;
}

void Room::HandleMove(Protocol::C_MOVE pkt, const uint64 objectId)
{
	// TODO : validate

	auto iter = _players.find(objectId);
	if (iter == _players.end() || iter->second == nullptr)
		return;

	PlayerRef player = iter->second;

	player->moveInfo->CopyFrom(pkt.move_info());
	{
		Protocol::S_MOVE movePkt;
		{
			Protocol::PlayerInfo* info = movePkt.mutable_player_info();
			info->set_object_id(objectId);
			Protocol::MoveInfo* moveInfo = info->mutable_move_info();
			moveInfo->CopyFrom(pkt.move_info());
		}

		SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(movePkt);
		Broadcast(sendBuffer);
	}
}

void Room::HandleFire(PlayerRef player, Protocol::C_FIRE pkt)
{
	if (player->GetObjectId() != pkt.object_id())
		return;

	if (_players.find(player->GetObjectId()) == _players.end())
		return;

	if (!player->fireFlag)
		return;

	player->fireFlag = false;
	player->fireTimer = 10;

	Protocol::S_FIRE firePkt;
	firePkt.set_object_id(pkt.object_id());
	firePkt.set_spawn_x(pkt.spawn_x());
	firePkt.set_spawn_y(pkt.spawn_y());
	firePkt.set_spawn_z(pkt.spawn_z());
	firePkt.set_direction_x(pkt.direction_x());
	firePkt.set_direction_y(pkt.direction_y());
	firePkt.set_direction_z(pkt.direction_z());

	SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(firePkt);
	Broadcast(sendBuffer);
}

void Room::HandleHit(PlayerRef player, Protocol::C_HIT pkt)
{
	if (player == nullptr)
		return;

	if (_players.find(pkt.target_object_id()) == _players.end())
		return;

	if (_players.find(player->GetObjectId()) == _players.end())
		return;

	Protocol::MatchPlayerState& playerState = _players[player->GetObjectId()]->matchPlayerState;
	Protocol::MatchPlayerState& targetPlayerState = _players[pkt.target_object_id()]->matchPlayerState;
	
	if (_roomInfo.state() != Protocol::ROOM_STATE_PLAYING)
		return;

	if (_remainSeconds > 55) // 시작 대기 5초
		return;

	if (!targetPlayerState.is_alive())
		return;
	
	if(player->team == _players[pkt.target_object_id()]->team)
		return;

	targetPlayerState.set_hp(targetPlayerState.hp() - 10);
	if (targetPlayerState.hp() <= 0)
	{
		playerState.set_kill_count(playerState.kill_count() + 1);
		targetPlayerState.set_death_count(targetPlayerState.death_count() + 1);
		targetPlayerState.set_is_alive(false);
		targetPlayerState.set_hp(0);

		if (player->team == Protocol::TEAM_RED)
		{
			_matchInfo.set_red_score(_matchInfo.red_score() + 1);
		}
		else if (player->team == Protocol::TEAM_BLUE)
		{
			_matchInfo.set_blue_score(_matchInfo.blue_score() + 1);
		}

		

		Protocol::S_MATCH_STATE matchStatePkt;
		auto matchState = matchStatePkt.mutable_match_state();
		matchState->set_match_id(_matchInfo.match_id());
		matchState->set_remaining_time_seconds(_remainSeconds);
		matchState->set_red_score(_matchInfo.red_score());
		matchState->set_blue_score(_matchInfo.blue_score());

		Broadcast(ClientPacketHandler::MakeSendBuffer(matchStatePkt));
	}	

	{
		Protocol::S_PLAYER_STATE playerStatePkt;
		playerStatePkt.set_object_id(player->GetObjectId());
		playerStatePkt.mutable_player_state()->CopyFrom(playerState);

		SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(playerStatePkt);
		Broadcast(sendBuffer);
	}

	{
		Protocol::S_PLAYER_STATE targetplayerStatePkt;
		targetplayerStatePkt.set_object_id(pkt.target_object_id());
		targetplayerStatePkt.mutable_player_state()->CopyFrom(targetPlayerState);

		SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(targetplayerStatePkt);
		Broadcast(sendBuffer);
	}

	if(!targetPlayerState.is_alive())
		PlayerDespawn(pkt.target_object_id());
}

void Room::HandlePrepareMatch(PlayerRef player)
{
	if (player == nullptr)
		return;

	{
		WRITE_LOCK;

		if (_roomInfo.state() != Protocol::ROOM_STATE_LOADING)
			return;

		const uint64 objectId = player->GetObjectId();

		if (_players.find(objectId) == _players.end())
			return;

		player->isPrepareMatch = true;

		for (auto iter : _players)
		{
			if (iter.second->isPrepareMatch == false)
				return;
		}

		_roomInfo.set_state(Protocol::ROOM_STATE_PLAYING);


		for (auto iter : _players)
			iter.second->isPrepareMatch = false;
	}

	Protocol::S_MATCH_START matchStartPkt;
	matchStartPkt.set_match_id(_roomInfo.room_id());

	SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(matchStartPkt);
	Broadcast(sendBuffer);

	_remainSeconds = 60;
	_sendRemainSecondsTimer = 10;
	DoTimer(100, &Room::UpdateTick);
}

void Room::PlayerDespawn(uint64 objectId)
{
	if (_roomInfo.state() != Protocol::ROOM_STATE_PLAYING)
		return;

	Protocol::S_PLAYER_DESPAWN pkt;

	pkt.set_object_id(objectId);
	Broadcast(ClientPacketHandler::MakeSendBuffer(pkt));

	DoTimer(5000, &Room::PlayerRespawn, objectId);
}

void Room::PlayerRespawn(uint64 objectId)
{
	if (_roomInfo.state() != Protocol::ROOM_STATE_PLAYING)
		return;

	Protocol::S_PLAYER_RESPAWN pkt;

	PlayerRef player = _players[objectId];

	Protocol::PlayerInfo* playerInfo = pkt.mutable_player_info();
	playerInfo->set_object_id(objectId);
	playerInfo->set_nickname(player->GetNickname());
	Protocol::MoveInfo* moveInfo = playerInfo->mutable_move_info();

	if (_players.find(objectId) == _players.end())
		return;

	moveInfo->set_state(Protocol::MOVE_STATE_IDLE);
	switch (player->team)
	{
	case Protocol::TEAM_BLUE:
		moveInfo->set_x(-1300.f);
		moveInfo->set_y(-1300.f);
		moveInfo->set_z(100.f);
		break;
	case Protocol::TEAM_RED:
		moveInfo->set_x(1300.f);
		moveInfo->set_y(1300.f);
		moveInfo->set_z(100.f);
		break;
	}
	moveInfo->set_yaw(0);
	player->moveInfo->CopyFrom(*moveInfo);

	Protocol::MatchPlayerState* state = pkt.mutable_player_state();
	player->matchPlayerState.set_hp(player->matchPlayerState.max_hp());
	player->matchPlayerState.set_is_alive(true);
	state->CopyFrom(player->matchPlayerState);

	Broadcast(ClientPacketHandler::MakeSendBuffer(pkt));
}

void Room::HandleReturnRoom(GameSessionRef session)
{
	Protocol::RoomInfo snapShot;

	{
		WRITE_LOCK;
		if (_roomInfo.state() == Protocol::ROOM_STATE_RESULT)
		{
			for (int32 i = 0; i < _roomInfo.players().size(); i++)
			{
				_roomInfo.mutable_players(i)->set_ready(false);
			}

			for (auto& [objectId, player] : _players)
			{
				player->isPrepareMatch = false;
			}

			_roomInfo.set_state(Protocol::ROOM_STATE_WAITING);
		}

		snapShot.CopyFrom(_roomInfo);
	}

	Protocol::S_RETURN_TO_ROOM pkt;
	pkt.mutable_room_info()->CopyFrom(snapShot);

	SEND_PACKET(pkt);
}

void Room::UpdateTick()
{
	if (_isClosing)
		return;
	
	for (auto& [playerId, player] : _players)
	{
		if (player == nullptr)
			continue;

		if (--player->fireTimer <= 0)
		{
			player->fireFlag = true;
			player->fireTimer = 10;
		}		
	}
	
	if (--_sendRemainSecondsTimer <= 0)
	{
		if (_remainSeconds <= 0)
		{
			_roomInfo.set_state(Protocol::ROOM_STATE_RESULT);

			Protocol::S_MATCH_END matchEndPkt;
			Protocol::MatchResult* matchResult = matchEndPkt.mutable_result();

			matchResult->set_match_id(_matchInfo.match_id());
			if (_matchInfo.red_score() > _matchInfo.blue_score())
			{
				matchResult->set_winner_team(Protocol::TEAM_RED);
			}
			else if (_matchInfo.red_score() < _matchInfo.blue_score())
			{
				matchResult->set_winner_team(Protocol::TEAM_BLUE);
			}
			else
				matchResult->set_winner_team(Protocol::TEAM_NONE);

			matchResult->set_red_score(_matchInfo.red_score());
			matchResult->set_blue_score(_matchInfo.blue_score());

			for (auto iter : _players)
			{
				matchResult->add_player_results()->CopyFrom(iter.second->matchPlayerState);
			}

			SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(matchEndPkt);
			Broadcast(sendBuffer);

			return;
		}
		WRITE_LOCK;
		_remainSeconds--;
		_sendRemainSecondsTimer = 10;

		Protocol::S_MATCH_STATE matchStatePkt;
		Protocol::MatchStateInfo* matchStateInfo = matchStatePkt.mutable_match_state();
		matchStateInfo->set_match_id(_roomInfo.room_id());
		matchStateInfo->set_remaining_time_seconds(_remainSeconds);
		matchStateInfo->set_red_score(_matchInfo.red_score());
		matchStateInfo->set_blue_score(_matchInfo.blue_score());

		SendBufferRef sendBuffer = ClientPacketHandler::MakeSendBuffer(matchStatePkt);
		Broadcast(sendBuffer);
	}

	DoTimer(100, &Room::UpdateTick);
}

RoomRef Room::GetRoomRef()
{
	return static_pointer_cast<Room>(shared_from_this());
}

bool Room::EnterPlayer(PlayerRef player)
{
	// 방이 삭제된 경우
	if (_isClosing)
		return false;

	// 있다면 문제가 있다.
	if (_players.find(player->GetObjectId()) != _players.end())
		return false;
	
	// 이미 다른방에 소속됨
	if (!player->room.load().expired())
		return false;

	// 방 다 참
	if (_players.size() >= _roomInfo.max_player_count())
		return false;

	_players.insert(make_pair(player->GetObjectId(), player));

	player->room.store(GetRoomRef());

	return true;
}


bool Room::LeavePlayer(uint64 objectId)
{
	// 없다면 문제가 있다.
	if (_players.find(objectId) == _players.end())
		return false;
	
	int32 playerIndex = -1;

	WRITE_LOCK;
	for (int i = 0; i < _roomInfo.players_size(); ++i)
	{
		if (_roomInfo.players(i).object_id() == objectId)
		{
			playerIndex = i;
			break;
		}
	}

	if (playerIndex == -1)
		return false;

	PlayerRef player = _players[objectId];

	const bool hasHost = _roomInfo.host_object_id() == player->GetObjectId();

	// 룸 정보에서 플레이어 삭제
	_roomInfo.mutable_players()->DeleteSubrange(playerIndex, 1);
	// 플레이어에게서 룸 삭제
	player->room.store(weak_ptr<Room>());
	player->matchPlayerState.Clear();
	player->team = Protocol::TEAM_NONE;
	// 룸에서 플레이어 삭제
	_players.erase(objectId);

	if (_players.empty())
	{
		_roomInfo.clear_host_object_id();
	}
	else if (hasHost)
	{
		_roomInfo.set_host_object_id(_roomInfo.players(0).object_id());
	}

	return true;
}

void Room::Broadcast(SendBufferRef sendBuffer, uint64 exceptId)
{
	for (auto& item : _players)
	{
		PlayerRef player = item.second;
		if (player->GetObjectId() == exceptId)
			continue;

		if (GameSessionRef session = player->session.lock())
			session->Send(sendBuffer);
	}
}

void RoomManager::HandleRoomList(GameSessionRef session)
{
	PlayerRef player = session->player.load();

	Protocol::S_ROOM_LIST roomListPkt;

	for(auto room : _rooms)
	{
		roomListPkt.add_rooms()->CopyFrom(room.second->GetRoomInfo());
	}

	SEND_PACKET(roomListPkt);
}

void RoomManager::HandleCreateRoom(GameSessionRef session, Protocol::C_CREATE_ROOM pkt)
{
	PlayerRef player = session->player.load();

	Protocol::S_CREATE_ROOM createRoomPkt;

	if (player == nullptr)
	{
		createRoomPkt.set_success(false);
		SEND_PACKET(createRoomPkt);
		return;
	}

	RoomRef room = MakeShared<Room>(session, CreateRoomId(), pkt.room_name(), pkt.max_player_count());
	
	if (!room->HandleEnterPlayer(player))
	{
		createRoomPkt.set_success(false);
		SEND_PACKET(createRoomPkt);
		return;
	}

	auto [iter, inserted] = _rooms.emplace(room->GetRoomId(), room);
	if (!inserted)
	{
		createRoomPkt.set_success(false);
		SEND_PACKET(createRoomPkt);
		return;
	}

	createRoomPkt.mutable_room_info()->CopyFrom(room->GetRoomInfo());
	createRoomPkt.set_success(true);

	SEND_PACKET(createRoomPkt);

	cout << player->GetObjectId() << " Create Room" << endl;
}

void RoomManager::HandleEnterRoom(GameSessionRef session, Protocol::C_ENTER_ROOM pkt)
{
	
	auto roomId = pkt.room_id();

	auto iter = _rooms.find(roomId);

	if (iter == _rooms.end())
	{
		Protocol::S_ENTER_ROOM enterRoomPkt;
		enterRoomPkt.set_success(false);
		SEND_PACKET(enterRoomPkt);
		return;
	}

	RoomRef room = iter->second;

	room->DoAsync([session, room](){

		Protocol::S_ENTER_ROOM enterRoomPkt;

		PlayerRef player = session->player.load();
		if (player == nullptr)
		{
			enterRoomPkt.set_success(false);
			SEND_PACKET(enterRoomPkt);
			return;
		}

		if (!room->HandleEnterPlayer(player))
		{
			enterRoomPkt.set_success(false);
			SEND_PACKET(enterRoomPkt);
			return;
		}

		enterRoomPkt.mutable_room_info()->CopyFrom(room->GetRoomInfo());
		enterRoomPkt.set_success(true);

		SEND_PACKET(enterRoomPkt);
		
		cout << session->player.load()->GetObjectId() << " Enter Room " << room->GetRoomInfo().room_id() << endl;
	});

}

void RoomManager::RemoveRoom(uint64 roomId)
{
	_rooms.erase(roomId);
}

uint64 RoomManager::CreateRoomId()
{

	return _roomIdGenerator.fetch_add(1);
}
