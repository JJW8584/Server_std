#pragma once

class Session;

enum class EventType : uint8
{
	Connect,
	Accept,
	//PreRecv, 0 byte recv
	Recv,
	Send
};

//--------------
//	IocpEvent
//--------------

// ������ 0���� OVERLAPPED�� �����Ƿ�
// IocpEvent �����ͳ� OVERLAPPED �����ͳ� ��� ����
class IocpEvent : public OVERLAPPED
{
public:
	// virtual ��� X, 0�� �޸𸮿� �ٸ� ���� �� �� ����
	IocpEvent(EventType type);

	void		Init();
	EventType	GetType() { return _type; }

protected:
	EventType	_type;
};

//-----------------
//	ConnectEvent
//-----------------

class ConnectEvent : public IocpEvent
{
public:
	ConnectEvent() : IocpEvent(EventType::Connect) {}
};

//---------------
//	AcceptEvent
//---------------

class AcceptEvent : public IocpEvent
{
public:
	AcceptEvent() : IocpEvent(EventType::Accept) {}

	void		SetSession(Session* session) { _session = session; }
	Session*	GetSession() { return _session; }

private:
	// Accept�� �� �߰������� �ʿ��� ���ڰ� ���� �� ����
	// Ŭ���̾�Ʈ ����
	Session*	_session = nullptr;
};

//--------------
//	RecvEvent
//--------------

class RecvEvent : public IocpEvent
{
public:
	RecvEvent() : IocpEvent(EventType::Recv) {}
};

//--------------
//	SendEvent
//--------------

class SendEvent : public IocpEvent
{
public:
	SendEvent() : IocpEvent(EventType::Send) {}
};