#pragma once

// Iocp에 등록할 수 있는 객체
//---------------
//	IocpObject
//---------------

class IocpObject
{
public:
	virtual HANDLE GetHandle() abstract;
	virtual void Dispatch(class IocpEvent* iocpEvent, int32 numOfByte = 0) abstract;
};

//-------------
//	IocpCore
//-------------

class IocpCore
{
public:
	IocpCore();
	~IocpCore();

	HANDLE		GetHandle() { return _iocpHandle; };

	// 세션, 소켓 생성시 iocp에 등록하는 함수
	bool		Register(class IocpObject* iocpObject);
	// iocp의 일감 탐지
	bool		Dispatch(uint32 timeoutMs = INFINITE);

private:
	HANDLE		_iocpHandle;
};

// TEMP
extern IocpCore	GIocpCore;