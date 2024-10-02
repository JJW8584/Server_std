#pragma once
#include "IocpCore.h"
#include "NetAddress.h"

class AcceptEvent;

//-------------
//	Listener
//-------------

// IocpCore에 등록 -> 얘를 잘 살펴봐
class Listener : public IocpObject
{
public:
	Listener() = default;
	~Listener();

public:
	/* 외부에서 사용 */
	// ex)식당 영업 개시할 때 문지기해라!, 어떤 주소를 대상으로 영업을 할지 알려줌
	bool StartAccept(NetAddress netAddress);
	void CloseSocket();

public:
	/* 인터페이스 구현 */
	virtual HANDLE GetHandle() override;
	virtual void Dispatch(class IocpEvent* iocpEvent, int32 numOfByte = 0) override;

private:
	/* 수신 관련 코드 */
	void RegisterAccept(AcceptEvent* acceptEvent);
	void ProcessAccept(AcceptEvent* acceptEvent);

protected:
	SOCKET _socket = INVALID_SOCKET;
	Vector<AcceptEvent*> _acceptEvents;
};

