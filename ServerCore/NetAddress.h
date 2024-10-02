#pragma once

// 클라이언트의 주소를 추출할 일이 있음
// 매번 함수를 부르기 보다 이 클래스로 매핑해서
// 편하게 사용하도록 함
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
	// DNS 주소를 ip주소로 바꾸는 등 기능 확장

private:
	SOCKADDR_IN _sockAddr = {};
};