// TabPageServer.cpp : implementation file
//

#include "stdafx.h"
#include "DeployTool.h"
#include "TabPageServer.h"
#include "SocketHelper.h"
#include <sstream>

#define MAXRECV 247815

#define DBOUT( s )            \
{                             \
   std::stringstream os_;    \
   os_ << s;                  \
   OutputDebugString( os_.str().c_str() );  \
}

// CTabPageServer dialog

unsigned int __stdcall ServerConnectThread_wrapper(void* data);
unsigned int __stdcall ServerUpdateThread_wrapper(void* data);

IMPLEMENT_DYNAMIC(CTabPageServer, CDialog)
CTabPageServer::CTabPageServer(CWnd* pParent /*=NULL*/)
	: CDialog(CTabPageServer::IDD, pParent)
{
	m_RecvBuffer = new char[MAXRECV];
	m_SendBuffer = new char[MAXRECV];
	m_DeployPort = 5000;
}

CTabPageServer::~CTabPageServer()
{
}

void CTabPageServer::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CTabPageServer, CDialog)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()


// CTabPageServer message handlers

void CTabPageServer::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here

}

void CTabPageServer::OnShowWindow(BOOL bShow, UINT nStatus)
{
}

void CTabPageServer::Startup()
{
	m_fExitThread = false;

	// startup server thread
	SocketHelper::CreateServerSocket(m_hServerSocket, m_DeployPort);

	// start thread to connect to clients
	HANDLE ServerConnectThreadHandle = (HANDLE)_beginthreadex(0, 0, &ServerConnectThread_wrapper, this, 0, 0);
	SetThreadPriority(ServerConnectThreadHandle, THREAD_PRIORITY_NORMAL);

	// start thread to update
	HANDLE ServerUpdateThreadHandle = (HANDLE)_beginthreadex(0, 0, &ServerUpdateThread_wrapper, this, 0, 0);
	SetThreadPriority(ServerUpdateThreadHandle, THREAD_PRIORITY_NORMAL);
}

void CTabPageServer::Shutdown()
{
	m_fExitThread = true;

	// Close the socket and mark as 0 in list for reuse
	closesocket(m_hServerSocket);
}

unsigned int __stdcall ServerConnectThread_wrapper(void* data)
{
    static_cast<CTabPageServer*>(data)->ServerConnectThread();
	return 0;
}

void CTabPageServer::ServerConnectThread()
{
	while (!m_fExitThread)
	{
		SOCKET hClientSocket;

		// listen for clients
		if (SocketHelper::WaitForClient(m_hServerSocket, hClientSocket))
		{
			m_hClients.push_back( hClientSocket );
		}
	}
}

unsigned int __stdcall ServerUpdateThread_wrapper(void* data)
{
    static_cast<CTabPageServer*>(data)->ServerUpdateThread();
	return 0;
}

void CTabPageServer::ServerUpdateThread()
{
	while (!m_fExitThread)
	{
		// update clients
	}
}