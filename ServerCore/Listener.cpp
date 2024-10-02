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

	// ���� �ɼ� ����
	if (SocketUtils::SetReuseAddress(_socket, true) == false)
		return false;

	if (SocketUtils::SetLinger(_socket, 0, 0) == false)
		return false;

	if (SocketUtils::BindAnyAddress(_socket, netAddress.Getport()) == false)
		return false;

	if (SocketUtils::Listen(_socket) == false)
		return false;

	// Ŭ���̾�Ʈ�� ������ �ٷ� �����ϸ� �ٷ� �Ϸ�� ���� �ְ�
	// �ƴϸ� ���߿� iocp�� ���� ��Ŀ ��������� �����ϴٰ� �Ϸ�� ���� ����
	// 1���� �ɾ��ְ� �Ǹ� �����ڰ� ������ �� �� ���� �������� ���� �� ����
	// �׷��� �������� �ΰ� �̺�Ʈ�� �ɾ���� ��
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
