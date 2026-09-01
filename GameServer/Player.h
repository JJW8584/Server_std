#pragma once

class GameSesison;
class Room;

class Player : public enable_shared_from_this<Player>
{
public:
	Player();
	~Player();

public:
	uint64 GetObjectId() const { return _objectId; }
	void SetObjectId(uint64 objectId) { _objectId = objectId; }

	const string& GetNickname() const { return _nickname; }

	void SetNickname(const string& nickname) { _nickname = nickname; }


public:
	atomic<weak_ptr<Room>> room;
	weak_ptr<GameSession> session; // cycle

	Protocol::MoveInfo* moveInfo;

	int32 fireTimer = 10;
	bool fireFlag = true;

public:
	bool isPrepareMatch = false;

	Protocol::Team team = Protocol::TEAM_NONE;
	Protocol::MatchPlayerState matchPlayerState;

private:
	uint64 _objectId = 0;
	string _nickname;
};