// TabPageClient.cpp : implementation file
//

#include "stdafx.h"
#include "DeployTool.h"
#include "TabPageClient.h"
#include "SocketHelper.h"
#include <sstream>

#define MAXRECV 247815

#define DBOUT( s )            \
{                             \
   std::stringstream os_;    \
   os_ << s;                  \
   OutputDebugString( os_.str().c_str() );  \
}

// CTabPageClient dialog

unsigned int __stdcall ClientThread_wrapper(void* data);

IMPLEMENT_DYNAMIC(CTabPageClient, CDialog)
CTabPageClient::CTabPageClient(CWnd* pParent /*=NULL*/)
	: CDialog(CTabPageClient::IDD, pParent)
{
	m_fConnected = false;
	m_RecvBuffer = new char[MAXRECV];
	m_strDeployServer = "127.0.0.1";
	m_DeployPort = 5000;
}

CTabPageClient::~CTabPageClient()
{
}

void CTabPageClient::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CTabPageClient, CDialog)
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()

// CTabPageClient message handlers

void CTabPageClient::OnShowWindow(BOOL bShow, UINT nStatus)
{
}

void CTabPageClient::Startup()
{
	m_fExitThread = false;

	// start thread to listen for client
	HANDLE ClientThreadHandle = (HANDLE)_beginthreadex(0, 0, &ClientThread_wrapper, this, 0, 0);
	SetThreadPriority(ClientThreadHandle, THREAD_PRIORITY_NORMAL);
}

void CTabPageClient::Shutdown()
{
	m_fExitThread = true;

	// Close the socket and mark as 0 in list for reuse
	closesocket(m_hSocket);
	m_fConnected = false;
}

unsigned int __stdcall ClientThread_wrapper(void* data)
{
    static_cast<CTabPageClient*>(data)->ClientThread();
	return 0;
}

void CTabPageClient::ClientThread()
{
	while (!m_fExitThread)
	{
		// keep trying to connect if we aren't already
		if (!m_fConnected)
		{
			m_fConnected = SocketHelper::ConnectToServer(m_hSocket, m_DeployPort, m_strDeployServer.c_str());
			if (m_fConnected)
			{
				//strConnectedHost[i] = strDestinationHost[i];
			}
		}

		// if we are connected wait for a command from the deployment server
		if (m_fConnected)
		{
			//Check if it was for closing , and also read the incoming message
			//recv does not place a null terminator at the end of the string (whilst printf %s assumes there is one).
			int result = recv(m_hSocket, m_RecvBuffer, MAXRECV, 0);
                 
			if(result == SOCKET_ERROR || result == 0)
			{
				int error_code = WSAGetLastError();
				if(error_code == WSAECONNRESET || result == 0)
				{
					DBOUT( "Host disconnected unexpectedly" << "" );
				}
				else
				{
					DBOUT( "Recv failed with error code:" << error_code );
				}

				//Close the socket and mark as 0 in list for reuse
				closesocket(m_hSocket);
				m_fConnected = false;
			}
			else
			{
				// parse command
			}
		}
	}
}