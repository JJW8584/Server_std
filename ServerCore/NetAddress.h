#pragma once

// Ŭ���̾�Ʈ�� �ּҸ� ������ ���� ����
// �Ź� �Լ��� �θ��� ���� �� Ŭ������ �����ؼ�
// ���ϰ� ����ϵ��� ��
//---------------
//	NetAddress
//---------------
class NetAddress
{
public:
	NetAddress() = default;
	NetAddress(SOCKADDR_IN sockAddr);
	NetAddress(wstring ip, uint64 port);

	SOCKADDR_IN&	GetSockAddr() { return _sockAddr; };
	wstring			GetIpAddr();
	uint64			Getport() { return ::ntohs(_sockAddr.sin_port); };

public:
	static IN_ADDR Ip2Address(const WCHAR* ip);
	// DNS �ּҸ� ip�ּҷ� �ٲٴ� �� ��� Ȯ��

private:
	SOCKADDR_IN _sockAddr = {};
};