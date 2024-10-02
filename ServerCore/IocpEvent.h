#pragma once

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

private:
	// Accept�� �� �߰������� �ʿ��� ���ڰ� ���� �� ����
	// TODO
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