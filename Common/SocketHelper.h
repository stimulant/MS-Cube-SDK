#pragma once
#include <winsock.h>

class SocketHelper
{
public:
	// Global
	static bool StartWinsock();
	static void StopWinsock();
	static void CloseConnection(SOCKET& hSocket);

	// Client
	static bool ConnectToServer(SOCKET& hSocket, int PortNo, const char* IPAddress);

	// Server
	static bool CreateServerSocket(SOCKET& hServerSocket, int PortNo);
	static bool WaitForClient(SOCKET& hServerSocket, SOCKET& hClientSocket);
};

