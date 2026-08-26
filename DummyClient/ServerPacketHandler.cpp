#include "pch.h"
#include "ServerPacketHandler.h"
#include "BufferReader.h"
#include "Protocol.pb.h"

PacketHandlerFunc GPacketHandler[UINT16_MAX];

// 컨텐츠 작업
bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len)
{
	PacketHeader* header = reinterpret_cast<PacketHeader*>(buffer);
	// TODO: Log

	return true;
}

bool Handle_S_LOGIN(PacketSessionRef& session, Protocol::S_LOGIN& pkt)
{
	if (pkt.success() == false)
		return true;

	if (pkt.players().size() == 0)
	{
		// 캐릭터 생성창
	}

	// 입장 UI 버튼 눌러서 게임 입장
	Protocol::C_ENTER_GAME enterGamePkt;
	enterGamePkt.set_playerindex(0); // 첫 번째 캐릭터로 입장
	auto sendBuffer = ServerPacketHandler::MakeSendBuffer(enterGamePkt);
	session->Send(sendBuffer);

	return true;
}

bool Handle_S_ENTER_GAME(PacketSessionRef& session, Protocol::S_ENTER_GAME& pkt)
{
	// TODO

	return true;
}

bool Handle_S_LEAVE_GAME(PacketSessionRef& session, Protocol::S_LEAVE_GAME& pkt)
{
	return false;
}

bool Handle_S_SPAWN(PacketSessionRef& session, Protocol::S_SPAWN& pkt)
{
	return false;
}

bool Handle_S_DESPAWN(PacketSessionRef& session, Protocol::S_DESPAWN& pkt)
{
	return false;
}

bool Handle_S_MOVE(PacketSessionRef& session, Protocol::S_MOVE& pkt)
{
	return false;
}

bool Handle_S_CHAT(PacketSessionRef& session, Protocol::S_CHAT& pkt)
{
	cout << pkt.msg() << endl;

	return true;
}
