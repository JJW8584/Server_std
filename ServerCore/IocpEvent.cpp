#include "pch.h"
#include "IocpEvent.h"

IocpEvent::IocpEvent(EventType type)
{
	Init();
}

void IocpEvent::Init()
{
	// 운영체제가 알아서 사용, 우리가 건들 일 없음
	OVERLAPPED::hEvent = 0;
	OVERLAPPED::Internal = 0;
	OVERLAPPED::InternalHigh = 0;
	OVERLAPPED::Offset = 0;
	OVERLAPPED::OffsetHigh = 0;
}
