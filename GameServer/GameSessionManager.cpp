#include "pch.h"
#include "GameSessionManager.h"
#include "GameSession.h"

GameSessionManager GSessionManager;

void GameSessionManager::Add(GameSessionRef session)
{
	WRITE_LOCK;
	_sessions.insert(session);
}

void GameSessionManager::Remove(GameSessionRef session)
{
	WRITE_LOCK;
	_sessions.erase(session);
}

void GameSessionManager::Broadcast(SendBufferRef sendBuffer)
{
	WRITE_LOCK;
	for (GameSessionRef session : _sessions)
	{
		session->Send(sendBuffer);
	}
}

bool GameSessionManager::TryLogin(uint64 accountId, GameSessionRef session)
{
    WRITE_LOCK;

    for (const GameSessionRef& existing : _sessions)
    {
        if (existing != session &&
            existing->accountId.load() == accountId)
        {
            return false;
        }
    }

    session->accountId.store(accountId);
    return true;
}
