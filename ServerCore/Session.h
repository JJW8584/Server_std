#pragma once
#include "IocpCore.h"
#include "IocpEvent.h"
#include "NetAddress.h"

//------------
//	Session
//------------

class Session : public IocpObject
{
public:
	Session();
	virtual ~Session();

public:
	/* 정보 관련 */
	void		SetNetAddress(NetAddress address) { _netAddress = address; }
	NetAddress	GetNetAddress() { return _netAddress; }
	SOCKET		GetSocket() { return _socket; }

public:
	/* 인터페이스 구현 */
	virtual HANDLE		GetHandle() override;
	virtual void		Dispatch(class IocpEvent* iocpEvent, int32 numOfByte = 0) override;


public:
	// send, recv 관련 버퍼 들어가야됨
	// TEMP
	char _recvBuffer[1000];

private:
	SOCKET			_socket = INVALID_SOCKET;
	NetAddress		_netAddress = {};
	Atomic<bool>	_connected = false;
};

