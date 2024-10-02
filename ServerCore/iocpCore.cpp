#include "pch.h"
#include "IocpCore.h"
#include "IocpEvent.h"

/*
	���ø��� ��Ʈ ���� -> �������͸� ���ؼ� ���
	-> ��Ŀ ��������� ����ġ�� �ϰ� ã�� ����
*/

// TEMP
IocpCore GIocpCore;

//-------------
//	IocpCore
//-------------

IocpCore::IocpCore()
{
	// �ڵ� ����
	_iocpHandle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	ASSERT_CRASH(_iocpHandle != INVALID_HANDLE_VALUE);
}

IocpCore::~IocpCore()
{
	::CloseHandle(_iocpHandle);
}

bool IocpCore::Register(IocpObject* iocpObject)
{
	// ù��° ���ڸ� �����ϰڴ�
	// iocpObject�� ������ ���ǰ� ����
	return ::CreateIoCompletionPort(iocpObject->GetHandle(), _iocpHandle, reinterpret_cast<ULONG_PTR>(iocpObject), 0);
}

// ��Ŀ ��������� Dispatch �����ϸ鼭 �ϰ� ã��
bool IocpCore::Dispatch(uint32 timeoutMs)
{
	DWORD numOfByte = 0;
	IocpObject* iocpObject = nullptr;
	IocpEvent* iocpEvent = nullptr;
	// ��� �� �ݵ�� ���۷��� ī�����ؾ���!

	// �ۼ��ŵ� ����Ʈ�� numOfByte�� �����
	if (::GetQueuedCompletionStatus(_iocpHandle, OUT & numOfByte, OUT reinterpret_cast<PULONG_PTR>(&iocpObject), OUT reinterpret_cast<LPOVERLAPPED*>(&iocpEvent), timeoutMs))
	{
		iocpObject->Dispatch(iocpEvent, numOfByte);
	}
	else
	{
		int32 errCode = ::WSAGetLastError();
		switch (errCode)
		{
		case WAIT_TIMEOUT:
			return false;
		default:
			// TODO : �α� ���
			iocpObject->Dispatch(iocpEvent, numOfByte);
			break;
		}
	}

	return false;
}
