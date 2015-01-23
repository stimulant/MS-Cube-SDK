// TabPageClient.cpp : implementation file
//

#include "stdafx.h"
#include <windows.h>
#include <process.h>
#include <Tlhelp32.h>
#include <winbase.h>
#include <string.h>
#include <sstream>
#include "DeployTool.h"
#include "TabPageClient.h"
#include "SocketHelper.h"
#include "DeployFile.h"

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

void killProcessByName(const char *filename)
{
    HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, NULL);
    PROCESSENTRY32 pEntry;
    pEntry.dwSize = sizeof (pEntry);
    BOOL hRes = Process32First(hSnapShot, &pEntry);
    while (hRes)
    {
        if (strcmp(pEntry.szExeFile, filename) == 0)
        {
            HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, 0,
                                          (DWORD) pEntry.th32ProcessID);
            if (hProcess != NULL)
            {
                TerminateProcess(hProcess, 9);
                CloseHandle(hProcess);
            }
        }
        hRes = Process32Next(hSnapShot, &pEntry);
    }
    CloseHandle(hSnapShot);
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
				char appname[MAX_PATH] = "";
				recv(mSocket, appname, MAX_PATH, 0);

				// send back YS if this app name is selected
				int curSel = this->SendDlgItemMessage(IDC_COMBO_APPS, CB_GETCURSEL, 0, 0);
				if (curSel == -1)
					send(mSocket, "NO", 2, 0);
				else
				{
					char pFilename[MAX_PATH];
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
					char appname[MAX_PATH] = "";
					recv(mSocket, appname, MAX_PATH, 0);
					mAppNames.push_back(appname);
					send(mSocket, "OK", 2, 0);

					this->SendDlgItemMessage(IDC_COMBO_APPS, CB_ADDSTRING, 0, (LPARAM)appname);
				}
			}
			if (strcmp(command, "ASKFILEUPDATE") == 0)
			{
				// check if a file needs an update
				if (!DeployFile::ClientDoesFileNeedUpdate(mSocket))
				{
					closesocket(mSocket);
					mConnected = false;
				}
			}
			if (strcmp(command, "SENDFILE") == 0)
			{
				// receive file
				if (!DeployFile::ClientReceiveFile(mSocket))
				{
					closesocket(mSocket);
					mConnected = false;
				}
			}
			if (strcmp(command, "KILLAPP") == 0)
			{
				// receive appName name
				char appName[MAX_PATH] = "";
				recv(mSocket, appName, MAX_PATH, 0);
				DBOUT("CLIENT: starting app: " << appName << "\n");
				send(mSocket, "OK", 2, 0);

				// receive executable name
				char executableName[MAX_PATH] = "";
				recv(mSocket, executableName, MAX_PATH, 0);
				send(mSocket, "OK", 2, 0);

				killProcessByName(executableName);
			}
			if (strcmp(command, "STARTAPP") == 0)
			{
				// receive appName name
				char appName[MAX_PATH] = "";
				recv(mSocket, appName, MAX_PATH, 0);
				DBOUT("CLIENT: starting app: " << appName << "\n");
				send(mSocket, "OK", 2, 0);

				// receive executable name
				char executableName[MAX_PATH] = "";
				recv(mSocket, executableName, MAX_PATH, 0);
				send(mSocket, "OK", 2, 0);

				std::string appPath = "";
				appPath += appName;
				appPath += "\\";
				appPath += executableName;

				STARTUPINFO info={sizeof(info)};
				PROCESS_INFORMATION processInfo;
				LPSTR s = const_cast<char *>(appPath.c_str());
				CreateProcess(s, s, NULL, NULL, TRUE, 0, NULL, NULL, &info, &processInfo);
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