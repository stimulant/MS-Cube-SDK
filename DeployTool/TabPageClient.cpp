// TabPageClient.cpp : implementation file
//

#include "stdafx.h"
#include "DeployTool.h"
#include "TabPageClient.h"
#include "SocketHelper.h"
#include "DeployFile.h"
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
	mConnected = false;
	mRecvBuffer = new char[MAXRECV];
	mStrDeployServer = "127.0.0.1";
	mDeployPort = 5000;
}

CTabPageClient::~CTabPageClient()
{
}

BOOL CTabPageClient::OnInitDialog()
{
   CDialog::OnInitDialog();

   SetDlgItemText(IDC_SERVER_EDIT, mStrDeployServer.c_str());
   return TRUE;  // return TRUE unless you set the focus to a control 
}

void CTabPageClient::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CTabPageClient, CDialog)
	ON_WM_SHOWWINDOW()
	ON_EN_CHANGE(IDC_SERVER_EDIT, OnChangeServerEdit)
END_MESSAGE_MAP()

// CTabPageClient message handlers

void CTabPageClient::OnShowWindow(BOOL bShow, UINT nStatus)
{
}

void CTabPageClient::OnChangeServerEdit()
{
	char serverHost[100];
	GetDlgItemText(IDC_SERVER_EDIT, serverHost, 100);
	mStrDeployServer = serverHost;
}

void CTabPageClient::Startup()
{
	mfExitThread = false;

	// start thread to listen for client
	HANDLE ClientThreadHandle = (HANDLE)_beginthreadex(0, 0, &ClientThread_wrapper, this, 0, 0);
	SetThreadPriority(ClientThreadHandle, THREAD_PRIORITY_NORMAL);
}

void CTabPageClient::Shutdown()
{
	mfExitThread = true;

	// Close the socket and mark as 0 in list for reuse
	closesocket(mSocket);
	mConnected = false;
}

unsigned int __stdcall ClientThread_wrapper(void* data)
{
    static_cast<CTabPageClient*>(data)->ClientThread();
	return 0;
}

void CTabPageClient::ClientThread()
{
	while (!mfExitThread)
	{
		// keep trying to connect if we aren't already
		if (!mConnected)
		{
			mConnected = SocketHelper::ConnectToServer(mSocket, mDeployPort, mStrDeployServer.c_str());
			if (mConnected)
			{
				//strConnectedHost[i] = strDestinationHost[i];
			}
		}

		// if we are connected wait for a command from the deployment server
		if (mConnected)
		{
			// receive command
			char command[32] = "";
			recv(mSocket, command, 32, 0);
			//DBOUT("CLIENT: received command: " << command << "\n");
			send(mSocket, "OK", 2, 0);

			if (strcmp(command, "ISAPPSELECTED") == 0)
			{
				// receive app name
				char appname[256] = "";
				recv(mSocket, appname, 256, 0);

				// send back YS if this app name is selected
				int curSel = this->SendDlgItemMessage(IDC_COMBO_APPS, CB_GETCURSEL, 0, 0);
				if (curSel == -1)
					send(mSocket, "NO", 2, 0);
				else
				{
					char pFilename[_MAX_PATH];
					this->SendDlgItemMessage(IDC_COMBO_APPS, CB_GETLBTEXT, curSel, (LPARAM)pFilename);

					if (strcmp(pFilename, appname) == 0)
						send(mSocket, "YS", 2, 0);
					else
						send(mSocket, "NO", 2, 0);
				}
			}
			if (strcmp(command, "LISTAPPS") == 0)
			{
				mAppNames.clear();

				// receive app count
				char rec[50] = "";
				int recs = recv(mSocket, rec, 10, 0);
				send(mSocket, "OK", 2, 0);
				rec[recs] = '\0';
				int appCount = atoi(rec);

				// receive app names
				for (int i=0; i<appCount; i++)
				{
					char appname[256] = "";
					recv(mSocket, appname, 256, 0);
					mAppNames.push_back(appname);
					send(mSocket, "OK", 2, 0);

					this->SendDlgItemMessage(IDC_COMBO_APPS, CB_ADDSTRING, 0, (LPARAM)appname);
				}
			}
			if (strcmp(command, "DOESFILENEEDUPDATE") == 0)
			{
				// check if a file needs an update
				if (!DeployFile::DoesFileNeedUpdate(mSocket))
				{
					closesocket(mSocket);
					mConnected = false;
				}
			}
			if (strcmp(command, "SENDFILE") == 0)
			{
				// receive file
				if (!DeployFile::ReceiveFile(mSocket))
				{
					closesocket(mSocket);
					mConnected = false;
				}
			}
			if (strcmp(command, "STARTAPP") == 0)
			{
				// receive executable name
				char executableName[128] = "";
				recv(mSocket, executableName, 128, 0);
				DBOUT("CLIENT: starting executable: " << executableName << "\n");
				send(mSocket, "OK", 2, 0);
				//DBOUT("CLIENT: sent executable ack\n");

				STARTUPINFO info={sizeof(info)};
				PROCESS_INFORMATION processInfo;
				CreateProcess(executableName, executableName, NULL, NULL, TRUE, 0, NULL, NULL, &info, &processInfo);
				/*
				if (CreateProcess(path, cmd, NULL, NULL, TRUE, 0, NULL, NULL, &info, &processInfo))
				{
					::WaitForSingleObject(processInfo.hProcess, INFINITE);
					CloseHandle(processInfo.hProcess);
					CloseHandle(processInfo.hThread);
				}*/
			}

			Sleep(100);
		}
	}
}