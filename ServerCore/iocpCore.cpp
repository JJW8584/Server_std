#include "pch.h"
#include "IocpCore.h"
#include "IocpEvent.h"

/*
	컴플리션 포트 만듦 -> 레지스터를 통해서 등록
	-> 워커 스레드들이 디스패치로 일감 찾고 실행
*/

// TEMP
IocpCore GIocpCore;

//-------------
//	IocpCore
//-------------

IocpCore::IocpCore()
{
	// 핸들 생성
	_iocpHandle = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	ASSERT_CRASH(_iocpHandle != INVALID_HANDLE_VALUE);
}

IocpCore::~IocpCore()
{
	::CloseHandle(_iocpHandle);
}

bool IocpCore::Register(IocpObject* iocpObject)
{
	// 첫번째 인자를 관찰하겠다
	// iocpObject의 역할이 세션과 같음
	return ::CreateIoCompletionPort(iocpObject->GetHandle(), _iocpHandle, reinterpret_cast<ULONG_PTR>(iocpObject), 0);
}

// 워커 스레드들이 Dispatch 실행하면서 일감 찾음
bool IocpCore::Dispatch(uint32 timeoutMs)
{
	DWORD numOfByte = 0;
	IocpObject* iocpObject = nullptr;
	IocpEvent* iocpEvent = nullptr;
	// 등록 시 반드시 레퍼런스 카운팅해야함!

	// 송수신된 바이트를 numOfByte에 뱉어줌
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
			// TODO : 로그 찍기
			iocpObject->Dispatch(iocpEvent, numOfByte);
			break;
		}
	}

	return false;
}
