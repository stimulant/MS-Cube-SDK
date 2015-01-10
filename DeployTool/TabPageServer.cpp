// TabPageServer.cpp : implementation file
//

#include "stdafx.h"
#include "DeployTool.h"
#include "TabPageServer.h"
#include "SocketHelper.h"
#include "DeployManager.h"
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
	mRecvBuffer = new char[MAXRECV];
	mSendBuffer = new char[MAXRECV];
	mDeployPort = 5000;
}

CTabPageServer::~CTabPageServer()
{
}

void CTabPageServer::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CTabPageServer, CDialog)
	ON_BN_CLICKED(IDC_ADDAPP, OnBnClickedAddApp)
	ON_BN_CLICKED(IDC_REMOVEAPP, OnBnClickedRemoveApp)
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()

// CTabPageServer message handlers

void CTabPageServer::OnBnClickedAddApp()
{
	OPENFILENAME ofn;       // common dialog box structure
	char szFile[260];       // buffer for file name
	
	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = this->m_hWnd;
	ofn.lpstrFile = szFile;
	// Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
	// use the contents of szFile to initialize itself.
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "All\0*.*\0Executable\0*.EXE\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	// Display the Open dialog box. 
	if (GetOpenFileName(&ofn)==TRUE)
	{
		DBOUT( "File: " << ofn.lpstrFile << "\n" );
		this->SendDlgItemMessageA(IDC_APPLIST, LB_ADDSTRING, 0, (LPARAM)ofn.lpstrFile);
	}
}

void CTabPageServer::OnBnClickedRemoveApp()
{
	// remove selected item
    int selection = this->SendDlgItemMessageA(IDC_APPLIST, LB_GETCURSEL, (WPARAM)0, (LPARAM)0);
	if (selection != LB_ERR)
		this->SendDlgItemMessageA(IDC_APPLIST, LB_DELETESTRING, (WPARAM)selection, (LPARAM)0);
}

void CTabPageServer::OnShowWindow(BOOL bShow, UINT nStatus)
{
}

void CTabPageServer::Startup()
{
	mfExitThread = false;

	// startup server thread
	SocketHelper::CreateServerSocket(mhServerSocket, mDeployPort);

	// start thread to connect to clients
	HANDLE ServerConnectThreadHandle = (HANDLE)_beginthreadex(0, 0, &ServerConnectThread_wrapper, this, 0, 0);
	SetThreadPriority(ServerConnectThreadHandle, THREAD_PRIORITY_NORMAL);

	// start thread to update
	HANDLE ServerUpdateThreadHandle = (HANDLE)_beginthreadex(0, 0, &ServerUpdateThread_wrapper, this, 0, 0);
	SetThreadPriority(ServerUpdateThreadHandle, THREAD_PRIORITY_NORMAL);

	// create DeployManager and create test app
	DeployManager::instance()->AddDeployApp("C:\\Users\\joel\\Desktop\\12_9_2014", "render_test.exe");
}

void CTabPageServer::Shutdown()
{
	mfExitThread = true;

	// Close the socket and mark as 0 in list for reuse
	closesocket(mhServerSocket);
}

unsigned int __stdcall ServerConnectThread_wrapper(void* data)
{
    static_cast<CTabPageServer*>(data)->ServerConnectThread();
	return 0;
}

void CTabPageServer::ServerConnectThread()
{
	while (!mfExitThread)
	{
		SOCKET hClientSocket;

		// listen for clients
		if (SocketHelper::WaitForClient(mhServerSocket, hClientSocket))
		{
			mhClients.push_back(hClientSocket);

			// send app list to client
			DeployManager::instance()->SendAppListToClient(hClientSocket);

			// send files to client
			DeployManager::instance()->SendToClient(hClientSocket);

			// start up the app
			DeployManager::instance()->StartApp(hClientSocket, "render_test.exe");
		}
		Sleep(100);
	}
}

unsigned int __stdcall ServerUpdateThread_wrapper(void* data)
{
    static_cast<CTabPageServer*>(data)->ServerUpdateThread();
	return 0;
}

void CTabPageServer::ServerUpdateThread()
{
	while (!mfExitThread)
	{
		// update clients
	}
}