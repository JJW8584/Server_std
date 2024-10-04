#include "pch.h"
#include "Listener.h"
#include "SocketUtils.h"
#include "IocpEvent.h"
#include "Session.h"


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
	ASSERT_CRASH(iocpEvent->GetType() == EventType::Accept);

	AcceptEvent
}

// iocp���� �̺�Ʈ�� ó���� �� �ֵ��� �ϰ��� ȣ���ϰ� �����ϴ� ����
// Register�� �̳��� �����ִ� �ܰ� (���� ���ο� ������ ����� ��)
// Process�� ���ô븦 ȸ���ؼ� ����⸦ �����ϴ� �ܰ�(������ �Ŵ����� ó��, ������ ���� �κ�)

// listener�� AcceptExtened�� ȣ���ϴ� ���� �ٽ�
void Listener::RegisterAccept(AcceptEvent* acceptEvent)
{
	// Ŭ�� ���ӽ� ���õ� ��� ������ ���ǿ� �� ����
	Session* session = Xnew<Session>();

	acceptEvent->Init();
	acceptEvent->SetSession(session);

	// acceptEvent�� session���� ����
	// �׷��� ���߿� Dispatch�ؼ� �̺�Ʈ�� �������� �� � ������ �Ѱ������ �� �� ����

	DWORD bytesReceived = 0;
	if (false == SocketUtils::AcceptEx(_socket, session->GetSocket(), session->_recvBuffer, 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, OUT & bytesReceived, static_cast<LPOVERLAPPED>(acceptEvent)))
	{
		const int32 errCode = ::WSAGetLastError();
		if (errCode != WSA_IO_PENDING/* ������ Ŭ�� ��� �׳� �������� ��Ȳ */)
		{
			// �ϴ� �ٽ� accept �ɾ���
			RegisterAccept(acceptEvent);
		}
	}
}

void Listener::ProcessAccept(AcceptEvent* acceptEvent)
{
}
