#include "stdafx.h"
#include "SocketHelper.h"

bool SocketHelper::StartWinsock()
{
	// Start winsock
    WSADATA wsadata;
    int error = WSAStartup(0x0202, &wsadata);
    if (error)
        return false;

    // Make sure we have winsock v2
    if (wsadata.wVersion != 0x0202)
    {
        WSACleanup();
        return false;
    }
	return true;
}

void SocketHelper::StopWinsock()
{
	WSACleanup();
}

bool SocketHelper::ConnectToHost(SOCKET& hSocket, int PortNo, const char* IPAddress)
{
    // Setup socket address
    SOCKADDR_IN target;
    target.sin_family = AF_INET;
    target.sin_port = htons (PortNo);
    target.sin_addr.s_addr = inet_addr(IPAddress);

	// Create socket
    hSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (hSocket == INVALID_SOCKET)
        return false;

    // Connect
    if (connect(hSocket, (SOCKADDR *)&target, sizeof(target)) == SOCKET_ERROR)
        return false;
    else
        return true;
}

void SocketHelper::CloseConnection(SOCKET& hSocket)
{
    if (hSocket)
        closesocket(hSocket);
}