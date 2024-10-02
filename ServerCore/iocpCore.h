#pragma once

// Iocp�� ����� �� �ִ� ��ü
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

	// ����, ���� ������ iocp�� ����ϴ� �Լ�
	bool		Register(class IocpObject* iocpObject);
	// iocp�� �ϰ� Ž��
	bool		Dispatch(uint32 timeoutMs = INFINITE);

private:
	HANDLE		_iocpHandle;
};

// TEMP
extern IocpCore	GIocpCore;