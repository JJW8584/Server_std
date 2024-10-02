#include "pch.h"
#include "Listener.h"
#include "SocketUtils.h"
#include "IocpEvent.h"


//-------------
//	Listener
//-------------

Listener::~Listener()
{
	SocketUtils::Close(_socket);

	for (AcceptEvent* acceptEvent : _acceptEvents)
	{
		// TODO
		
		Xdelete(acceptEvent);
	}
}

bool Listener::StartAccept(NetAddress netAddress)
{
	_socket = SocketUtils::CreateSocket();
	if (_socket == INVALID_SOCKET)
		return false;

	if (GIocpCore.Register(this) == false)
		return false;

	// 소켓 옵션 설정
	if (SocketUtils::SetReuseAddress(_socket, true) == false)
		return false;

	if (SocketUtils::SetLinger(_socket, 0, 0) == false)
		return false;

	if (SocketUtils::BindAnyAddress(_socket, netAddress.Getport()) == false)
		return false;

	if (SocketUtils::Listen(_socket) == false)
		return false;

	// 클라이언트가 운좋게 바로 접속하면 바로 완료될 수도 있고
	// 아니면 나중에 iocp를 통해 워커 스레드들이 관찰하다가 완료될 수도 있음
	// 1개만 걸어주게 되면 접속자가 몰렸을 때 몇 명은 접속하지 못할 수 있음
	// 그래서 여유분을 두고 이벤트를 걸어줘야 됨
	const int32 acceptCount = 1;
	for (int32 i = 0; i < acceptCount; i++)
	{
		AcceptEvent* acceptEvent = Xnew<AcceptEvent>();
		_acceptEvents.push_back(acceptEvent);
		RegisterAccept(acceptEvent);
	}

	return false;
}

void Listener::CloseSocket()
{
	SocketUtils::Close(_socket);
}

HANDLE Listener::GetHandle()
{
	return reinterpret_cast<HANDLE>(_socket);
}

void Listener::Dispatch(IocpEvent* iocpEvent, int32 numOfByte)
{
}

void Listener::RegisterAccept(AcceptEvent* acceptEvent)
{
}

void Listener::ProcessAccept(AcceptEvent* acceptEvent)
{
}
