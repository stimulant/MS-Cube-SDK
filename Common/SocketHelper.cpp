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

bool SocketHelper::ConnectToServer(SOCKET& hSocket, int PortNo, const char* IPAddress)
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

bool SocketHelper::CreateServerSocket(SOCKET& hServerSocket, int PortNo)
{
	// Setup socket address (listen on any port
	SOCKADDR_IN target;
    target.sin_family = AF_INET;
    target.sin_port = htons( PortNo );
	target.sin_addr.s_addr = INADDR_ANY;

	// Create a socket
    hServerSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (hServerSocket == INVALID_SOCKET)
        return false;
      
    // Bind
    if(bind(hServerSocket,(struct sockaddr *)&target , sizeof(target)) == SOCKET_ERROR)
	{
		closesocket(hServerSocket);
		return false;
	}
	return true;
}

bool SocketHelper::WaitForClient(SOCKET& hServerSocket, SOCKET& hClientSocket)
{  
    // Listen to incoming connections
	SOCKADDR_IN client;
    listen(hServerSocket, 1);
    int c = sizeof(struct sockaddr_in);
    hClientSocket = accept(hServerSocket, (struct sockaddr *)&client, &c);
    if (hClientSocket == INVALID_SOCKET)
	{
		closesocket(hServerSocket);
		return false;
	}
    return true;
}

void SocketHelper::CloseConnection(SOCKET& hSocket)
{
    if (hSocket)
        closesocket(hSocket);
}