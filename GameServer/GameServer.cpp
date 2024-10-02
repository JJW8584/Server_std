#include "pch.h"
#include "CorePch.h"
#include <Windows.h>
#include "ThreadManager.h"

// 소켓 유틸 사용 예시

// 자주 사용하지는 않지만 일일이 인자를 외우는 것보다
// 꼭 설정해야하는 인자만 두고 매핑하면 사용하기 편하다!
#include "SocketUtils.h"

int main()
{
	// 소켓 init과 clear는 CoreGlobal에서 수행
	SOCKET socket = SocketUtils::CreateSocket();

	SocketUtils::BindAnyAddress(socket, 7777);

	SocketUtils::Listen(socket);

	// 이후 IOCP와 연동해서 사용
	SOCKET clientSocket = ::accept(socket, nullptr, nullptr);

	cout << "Client Connected!" << endl;

	GThreadManager->Join();
}