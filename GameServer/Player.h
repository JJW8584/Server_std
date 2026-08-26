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

public:
	atomic<weak_ptr<Room>> room;
	weak_ptr<GameSession> session; // cycle

	Protocol::MoveInfo* moveInfo;

private:
	uint64 _objectId = 0;
};