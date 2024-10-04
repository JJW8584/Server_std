#include "pch.h"
#include "Session.h"
#include "SocketUtils.h"

//------------
//	Session
//------------

Session::Session()
{
	_socket = SocketUtils::CreateSocket();
}

Session::~Session()
{
	SocketUtils::Close(_socket);
}

HANDLE Session::GetHandle()
{
	return reinterpret_cast<HANDLE>(_socket);
}

// iocpEvent가 recv나 send같은 이벤트를 만들면 Dispatch가 처리
void Session::Dispatch(IocpEvent* iocpEvent, int32 numOfByte)
{
	// TODO
}
