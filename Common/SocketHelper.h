#pragma once
#include <winsock.h>

class SocketHelper
{
public:
	static bool StartWinsock();
	static void StopWinsock();

	static bool ConnectToServer(SOCKET& hSocket, int PortNo, const char* IPAddress);
	static bool WaitForClient(SOCKET& hServerSocket, SOCKET& hClientSocket, int PortNo);
	static void CloseConnection(SOCKET& hSocket);
};

