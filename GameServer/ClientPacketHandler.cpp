#include "pch.h"
#include "ClientPacketHandler.h"
#include "BufferReader.h"
#include "BufferWriter.h"
#include "ObjectUtils.h"
#include "Room.h"
#include "Player.h"
#include "DBConnectionPool.h"
#include "DBBind.h"
#include "FileUtils.h"
#include <sodium.h>
#include <cwctype>

PacketHandlerFunc GPacketHandler[UINT16_MAX];

// DB 클래스

namespace
{
	// 로그인
	class FindAccountQuery : public DBBind<1, 3>
	{
	public:
		explicit FindAccountQuery(DBConnection& connection)
			: DBBind(
				connection,
				L"SELECT account_id, password_hash, nickname "
				L"FROM accounts "
				L"WHERE login_id = ? "
				L"LIMIT 1")
		{
		}

		void InLoginId(const WCHAR* value)
		{
			BindParam(0, value);
		}

		void OutAccountId(int64& value)
		{
			BindCol(0, value);
		}

		template<int32 N>
		void OutPasswordHash(WCHAR(&value)[N])
		{
			BindCol(1, value);
		}

		template<int32 N>
		void OutNickname(WCHAR(&value)[N])
		{
			BindCol(2, value);
		}
	};

	std::string WideToUtf8(const WCHAR* value)
	{
		if (value == nullptr)
			return {};

		const int32 sourceLength = static_cast<int32>(::wcslen(value));

		if (sourceLength == 0)
			return {};

		// NULL 종료 문자를 제외한 실제 UTF-8 바이트 수 계산
		const int32 requiredSize = ::WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, value, sourceLength, nullptr, 0, nullptr, nullptr);

		if (requiredSize <= 0)
			return {};

		std::string result(requiredSize, '\0');

		const int32 convertedSize = ::WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, value, sourceLength, result.data(), requiredSize, nullptr, nullptr);

		if (convertedSize != requiredSize)
			return {};

		return result;
	}

	bool VerifyPassword(const std::string& password, const std::string& passwordHash)
	{
		return ::crypto_pwhash_str_verify(passwordHash.c_str(), password.data(), static_cast<unsigned long long>(password.size())) == 0;
	}

	// 회원가입
	class CheckRegisterDuplicateQuery : public DBBind<2, 2>
	{
	public:
		explicit CheckRegisterDuplicateQuery(DBConnection& connection)
			: DBBind(
				connection,
				L"SELECT "
				L"EXISTS(SELECT 1 FROM accounts WHERE login_id = ?), "
				L"EXISTS(SELECT 1 FROM accounts WHERE nickname = ?)")
		{
		}

		void InLoginId(const WCHAR* value)
		{
			BindParam(0, value);
		}

		void InNickname(const WCHAR* value)
		{
			BindParam(1, value);
		}

		void OutLoginIdExists(int32& value)
		{
			BindCol(0, value);
		}

		void OutNicknameExists(int32& value)
		{
			BindCol(1, value);
		}
	};

	class InsertAccountQuery : public DBBind<3, 0>
	{
	public:
		explicit InsertAccountQuery(DBConnection& connection)
			: DBBind(
				connection,
				L"INSERT INTO accounts "
				L"(login_id, password_hash, nickname) "
				L"VALUES (?, ?, ?)")
		{
		}

		void InLoginId(const WCHAR* value)
		{
			BindParam(0, value);
		}

		void InPasswordHash(const WCHAR* value)
		{
			BindParam(1, value);
		}

		void InNickname(const WCHAR* value)
		{
			BindParam(2, value);
		}
	};
}



// 컨텐츠 작업
bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len)
{
	return false;
}

bool Handle_C_LOGIN(PacketSessionRef& session, Protocol::C_LOGIN& pkt)
{
	Protocol::S_LOGIN loginPkt;
	loginPkt.set_success(false);
	loginPkt.set_result(Protocol::AUTH_RESULT_INVALID_CREDENTIALS);

	const std::string& loginId = pkt.login_id();
	const std::string& password = pkt.password();

	if (loginId.size() < 4 || loginId.size() > 64 || password.size() < 8 || password.size() > 128)
	{
		loginPkt.set_result(Protocol::AUTH_RESULT_INVALID_INPUT);

		SEND_PACKET(loginPkt);
		return true;
	}

	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);
	UINT64 userId;

	// 동일한 세션의 중복 로그인 방지
	if (gameSession->accountId.load() != 0)
	{
		loginPkt.set_result(Protocol::AUTH_RESULT_INVALID_INPUT);

		SEND_PACKET(loginPkt);
		return true;
	}

	String wideLoginId = FileUtils::Convert(loginId);

	int64 accountId = 0;
	WCHAR passwordHash[256] = {};
	WCHAR nickname[65] = {};

	DBConnection* connection = GDBConnectionPool->Pop();

	if (connection == nullptr)
	{
		loginPkt.set_result(Protocol::AUTH_RESULT_SERVER_ERROR);

		SEND_PACKET(loginPkt);
		return true;
	}

	bool querySucceeded = false;
	bool accountFound = false;

	{
		FindAccountQuery query(*connection);

		query.InLoginId(wideLoginId.c_str());
		query.OutAccountId(accountId);
		query.OutPasswordHash(passwordHash);
		query.OutNickname(nickname);

		querySucceeded = query.Execute();

		if (querySucceeded)
			accountFound = query.Fetch();
	}

	// 해시 검증 전에 DB 연결부터 반환
	GDBConnectionPool->Push(connection);

	if (!querySucceeded)
	{
		loginPkt.set_result(Protocol::AUTH_RESULT_SERVER_ERROR);

		SEND_PACKET(loginPkt);
		return true;
	}

	// 아이디가 없거나 비밀번호가 틀린 경우 같은 응답 사용
	if (!accountFound || !VerifyPassword(password, WideToUtf8(passwordHash)))
	{
		loginPkt.set_result(Protocol::AUTH_RESULT_INVALID_CREDENTIALS);

		SEND_PACKET(loginPkt);
		return true;
	}

	gameSession->accountId.store(static_cast<uint64>(accountId));
	gameSession->nickname = WideToUtf8(nickname);

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

	loginPkt.set_success(true);
	loginPkt.set_result(Protocol::AUTH_RESULT_SUCCESS);
	loginPkt.set_account_id(static_cast<uint64>(accountId));
	loginPkt.set_object_id(userId);
	loginPkt.set_nickname(gameSession->nickname);

	SEND_PACKET(loginPkt);

	cout << "Account " << accountId << " login success" << endl;

	return true;
}

bool Handle_C_REGISTER(PacketSessionRef& session, Protocol::C_REGISTER& pkt)
{
	Protocol::S_REGISTER registerPkt;
	registerPkt.set_success(false);
	registerPkt.set_result(Protocol::AUTH_RESULT_INVALID_INPUT);

	const std::string& loginId = pkt.login_id();
	const std::string& password = pkt.password();
	const std::string& nickname = pkt.nickname();

	// 아이디: 영문/숫자 4~64바이트
	if (loginId.size() < 4 || loginId.size() > 64)
	{
		SEND_PACKET(registerPkt);
		return true;
	}

	for (const unsigned char ch : loginId)
	{
		const bool isLetter = (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
		const bool isDigit = ch >= '0' && ch <= '9';

		if (!isLetter && !isDigit)
		{
			SEND_PACKET(registerPkt);
			return true;
		}
	}

	// 비밀번호: 8~128바이트
	if (password.size() < 8 || password.size() > 128)
	{
		SEND_PACKET(registerPkt);
		return true;
	}

	String wideLoginId = FileUtils::Convert(loginId);
	String wideNickname = FileUtils::Convert(nickname);

	// 닉네임: 2~16자
	if (wideNickname.size() < 2 || wideNickname.size() > 16)
	{
		SEND_PACKET(registerPkt);
		return true;
	}

	// 앞뒤 공백 및 제어문자 금지
	if (std::iswspace(wideNickname.front()) ||
		std::iswspace(wideNickname.back()))
	{
		SEND_PACKET(registerPkt);
		return true;
	}

	for (const WCHAR ch : wideNickname)
	{
		if (std::iswcntrl(ch))
		{
			SEND_PACKET(registerPkt);
			return true;
		}
	}

	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

	// 이미 로그인한 세션에서 회원가입 방지
	if (gameSession->accountId.load() != 0)
	{
		SEND_PACKET(registerPkt);
		return true;
	}

	// libsodium으로 비밀번호 해시 생성
	char passwordHash[crypto_pwhash_STRBYTES] = {};

	if (::crypto_pwhash_str(passwordHash, password.data(), static_cast<unsigned long long>(password.size()), crypto_pwhash_OPSLIMIT_INTERACTIVE, crypto_pwhash_MEMLIMIT_INTERACTIVE) != 0)
	{
		registerPkt.set_result(Protocol::AUTH_RESULT_SERVER_ERROR);

		SEND_PACKET(registerPkt);
		return true;
	}

	String widePasswordHash = FileUtils::Convert(std::string(passwordHash));

	DBConnection* connection = GDBConnectionPool->Pop();

	if (connection == nullptr)
	{
		registerPkt.set_result(
			Protocol::AUTH_RESULT_SERVER_ERROR);

		SEND_PACKET(registerPkt);
		return true;
	}

	int32 loginIdExists = 0;
	int32 nicknameExists = 0;

	bool duplicateQuerySucceeded = false;
	bool duplicateResultFetched = false;

	{
		CheckRegisterDuplicateQuery query(*connection);

		query.InLoginId(wideLoginId.c_str());
		query.InNickname(wideNickname.c_str());

		query.OutLoginIdExists(loginIdExists);
		query.OutNicknameExists(nicknameExists);

		duplicateQuerySucceeded = query.Execute();

		if (duplicateQuerySucceeded)
			duplicateResultFetched = query.Fetch();
	}

	if (!duplicateQuerySucceeded || !duplicateResultFetched)
	{
		GDBConnectionPool->Push(connection);

		registerPkt.set_result(Protocol::AUTH_RESULT_SERVER_ERROR);

		SEND_PACKET(registerPkt);
		return true;
	}

	if (loginIdExists != 0)
	{
		GDBConnectionPool->Push(connection);

		registerPkt.set_result(Protocol::AUTH_RESULT_DUPLICATE_LOGIN_ID);

		SEND_PACKET(registerPkt);
		return true;
	}

	if (nicknameExists != 0)
	{
		GDBConnectionPool->Push(connection);

		registerPkt.set_result(Protocol::AUTH_RESULT_DUPLICATE_NICKNAME);

		SEND_PACKET(registerPkt);
		return true;
	}

	bool insertSucceeded = false;

	{
		InsertAccountQuery query(*connection);

		query.InLoginId(wideLoginId.c_str());
		query.InPasswordHash(widePasswordHash.c_str());
		query.InNickname(wideNickname.c_str());

		insertSucceeded = query.Execute();
	}

	GDBConnectionPool->Push(connection);

	if (!insertSucceeded)
	{
		registerPkt.set_result(Protocol::AUTH_RESULT_SERVER_ERROR);

		SEND_PACKET(registerPkt);
		return true;
	}

	registerPkt.set_success(true);
	registerPkt.set_result(Protocol::AUTH_RESULT_SUCCESS);

	SEND_PACKET(registerPkt);

	cout << "Account registration success: " << loginId << endl;


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
	GameSessionRef gameSession = static_pointer_cast<GameSession>(session);

	PlayerRef player = gameSession->player.load();
	if (player == nullptr)
		return false;

	RoomRef room = player->room.load().lock();
	if (room == nullptr)
		return false;

	room->DoAsync(&Room::HandlePrepareMatch, player);

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

	// TODO : 유저 체크

	//room->HandleMove(pkt);

	room->DoAsync(&Room::HandleMove, pkt, player->GetObjectId());

	return true;
}

bool Handle_C_FIRE(PacketSessionRef& session, Protocol::C_FIRE& pkt)
{
	auto gameSession = static_pointer_cast<GameSession>(session);

	PlayerRef player = gameSession->player.load();
	if (player == nullptr)
		return false;

	RoomRef room = player->room.load().lock();
	if (room == nullptr)
		return false;

	room->DoAsync(&Room::HandleFire, player, pkt);

	return true;
}

bool Handle_C_HIT(PacketSessionRef& session, Protocol::C_HIT& pkt)
{
	auto gameSession = static_pointer_cast<GameSession>(session);

	PlayerRef player = gameSession->player.load();
	if (player == nullptr)
		return false;

	RoomRef room = player->room.load().lock();
	if (room == nullptr)
		return false;

	room->DoAsync(&Room::HandleHit, player, pkt);

	return true;
}

bool Handle_C_RETURN_TO_ROOM(PacketSessionRef& session, Protocol::C_RETURN_TO_ROOM& pkt)
{
	auto gameSession = static_pointer_cast<GameSession>(session);

	PlayerRef player = gameSession->player.load();
	if (player == nullptr)
		return false;

	RoomRef room = player->room.load().lock();
	if (room == nullptr)
		return false;

	room->DoAsync(&Room::HandleReturnRoom, gameSession);

	return true;
}

bool Handle_C_CHAT(PacketSessionRef& session, Protocol::C_CHAT& pkt)
{
	cout << pkt.msg() << endl;

	return true;
}
