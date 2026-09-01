#pragma once
#include "Session.h"

class Player;

class GameSession : public PacketSession
{
public:
	~GameSession()
	{
		cout << "~ServerSession" << endl;
	}

	virtual void OnConnected() override;
	virtual void OnDisconnected() override;
	virtual void OnRecvPacket(BYTE* buffer, int32 len) override;
	virtual void OnSend(int32 len) override;

public:
	atomic<uint64> accountId = 0;
	string nickname;
	atomic<shared_ptr<Player>> player;
};