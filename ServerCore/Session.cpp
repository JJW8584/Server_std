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

// iocpEvent�� recv�� send���� �̺�Ʈ�� ����� Dispatch�� ó��
void Session::Dispatch(IocpEvent* iocpEvent, int32 numOfByte)
{
	// TODO
}
