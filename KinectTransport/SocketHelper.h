#pragma once
#include <winsock.h>

class SocketHelper
{
public:
	static bool StartWinsock();
	static void StopWinsock();
	static bool ConnectToHost(SOCKET& hSocket, int PortNo, const char* IPAddress);
	static void CloseConnection(SOCKET& hSocket);
};

