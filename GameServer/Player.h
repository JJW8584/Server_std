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

	static constexpr float FIRE_COOLDOWN_SECONDS = 1.0f;
	float fireTimer = 0.0f;

public:
	bool isPrepareMatch = false;

	Protocol::MatchPlayerState matchPlayerState;

private:
	uint64 _objectId = 0;
};