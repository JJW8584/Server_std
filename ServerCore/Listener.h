#pragma once
#include "IocpCore.h"
#include "NetAddress.h"

class AcceptEvent;

//-------------
//	Listener
//-------------

// IocpCore�� ��� -> �긦 �� �����
class Listener : public IocpObject
{
public:
	Listener() = default;
	~Listener();

public:
	/* �ܺο��� ��� */
	// ex)�Ĵ� ���� ������ �� �������ض�!, � �ּҸ� ������� ������ ���� �˷���
	bool StartAccept(NetAddress netAddress);
	void CloseSocket();

public:
	/* �������̽� ���� */
	virtual HANDLE GetHandle() override;
	virtual void Dispatch(class IocpEvent* iocpEvent, int32 numOfByte = 0) override;

private:
	/* ���� ���� �ڵ� */
	void RegisterAccept(AcceptEvent* acceptEvent);
	void ProcessAccept(AcceptEvent* acceptEvent);

protected:
	SOCKET _socket = INVALID_SOCKET;
	Vector<AcceptEvent*> _acceptEvents;
};

