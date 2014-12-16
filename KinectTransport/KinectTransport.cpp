// Windows Header Files:
#include <windows.h>
#include <Windowsx.h>
#include <commctrl.h>
#include <Shellapi.h>
#include <Shlwapi.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include "resource.h"
#include <process.h>
#include <ctime>
#include "KinectData.h"
#include "KinectAPI.h"
#include "SocketHelper.h"
#include "RegistryHelper.h"

#define TRAYICONID	1//				ID number for the Notify Icon
#define SWM_TRAYMSG	WM_APP//		the message ID sent to our window

#define SWM_SHOW	WM_APP + 1//	show the window
#define SWM_EXIT	WM_APP + 2//	close the window

// Global Variables:
HINSTANCE		hInst;	// current instance
NOTIFYICONDATA	niData;	// notify icon data
HINSTANCE		hAppInstance;
HWND			hAppHwnd;
bool			fShouldExit;
bool			fShouldDisconnectKinect;

// socket connections
SOCKET			hSocket[4];
bool			fSocketConnected[4];
std::string		strDestinationHost[4];
std::string		strConnectedHost[4];
bool			fHostEnabled[4];
bool			fSendDepthData[4];
bool			fSendBodiesData[4];

// Kinect
KinectData *pKinectData;
unsigned int __stdcall KinectThread(void* data);
char* pDepthBinary = new char[495630];

// Forward declarations of functions included in this code module:
void				SetupIcon(bool connected, bool modify = true);
BOOL				InitInstance(HINSTANCE, int);
BOOL				OnInitDialog(HWND hWnd);
void				ShowContextMenu(HWND hWnd);
ULONGLONG			GetDllVersion(LPCTSTR lpszDllName);

INT_PTR CALLBACK	DlgProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	MSG msg;
	HACCEL hAccelTable;
	hAppInstance = hInstance;

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow)) return FALSE;
	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_KINECTTRANSPORT);

	// create Kinect data source
	pKinectData = new KinectData();
	fShouldExit = false;
	fShouldDisconnectKinect = false;
	for (int i=0; i<4; i++)
		fSocketConnected[i] = false;

	// start winsock
	SocketHelper::StartWinsock();

	// start thread to poll kinect and send data
	HANDLE kinectThreadHandle = (HANDLE)_beginthreadex(0, 0, &KinectThread, 0, 0, 0);
	SetThreadPriority(kinectThreadHandle, THREAD_PRIORITY_TIME_CRITICAL);

	// Main message loop:
	while (!fShouldExit)
	{
		while( ::PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) ) {
			::TranslateMessage( &msg );
			::DispatchMessage( &msg );
		}
	}

	// close kinect
	delete pKinectData;

	// stop winsock
	SocketHelper::StopWinsock();

	return (int) msg.wParam;
}

unsigned int __stdcall KinectThread(void* data)
{
	int bodyCount = 0;
	ULONG64 trackingIds[6] = {0};
	static std::map< JointType, std::array<float, 3> > jointPositions[6];

	while (!fShouldDisconnectKinect)
	{
		// try to connect if we aren't connected to all valid
		for (int i=0; i<4; i++)
		{
			if (fHostEnabled[i])
			{
				if (!fSocketConnected[i] || strDestinationHost[i] != strConnectedHost[i])
				{
					fSocketConnected[i] = SocketHelper::ConnectToServer(hSocket[i], 3000, strDestinationHost[i].c_str());
					if (fSocketConnected[i])
					{
						SetupIcon(true);
						strConnectedHost[i] = strDestinationHost[i];
					}
				}
			}
			else if (fSocketConnected[i])
			{
				SocketHelper::CloseConnection(hSocket[i]);
				fSocketConnected[i] = false;

				bool anyConnected = false;
				for (int i=0; i<4; i++)
					anyConnected |= fSocketConnected[i];
				if (!anyConnected)
					SetupIcon(false);
			}
		}

		// check if we should bother polling data at all
		bool getDepth = false, getBodies = false;
		for (int i=0; i<4; i++)
		{
			getBodies |= (fSocketConnected[i] && fHostEnabled[i] && fSendBodiesData[i]);
			getDepth |= (fSocketConnected[i] && fHostEnabled[i] && fSendDepthData[i]);
		}

		// retrieve and send body data
		if (getBodies)
		{
			bodyCount = 0;
			if (pKinectData->GetKinectBodies(trackingIds, jointPositions, bodyCount))
			{
				// turn bodies into a binary frame
				char binary[1208];
				int binarySize = KinectAPI::BodiesToBinary(trackingIds, jointPositions, bodyCount, binary);

				// send data out the sockets
				for (int i=0; i<4; i++)
				{
					if (fSocketConnected[i] && fHostEnabled[i] && fSendBodiesData[i])
					{
						if (send(hSocket[i], binary, binarySize, 0) == -1)
							fSocketConnected[i] = false;
					}
				}
			}
		}

		// retrieve and send depth data
		if (getDepth)
		{
			IDepthFrame* pDepthFrame = NULL;
			int nWidth = 0;
			int nHeight = 0;
			USHORT nMinDepth = 0;
			USHORT nMaxDepth = 0;
			UINT nBufferSize = 0;
			UINT16 *pBuffer = NULL;
			if (pKinectData->GetKinectDepth(&pDepthFrame, nWidth, nHeight, pBuffer, nMinDepth, nMaxDepth))
			{
				// turn depth into a binary frame
				int binarySize = KinectAPI::DepthToBinary(nWidth, nHeight, pBuffer, nMinDepth, nMaxDepth, pDepthBinary);

				for (int i=0; i<4; i++)
				{
					if (fSocketConnected[i] && fHostEnabled[i] && fSendDepthData[i])
					{
						if (send(hSocket[i], pDepthBinary, binarySize, 0) == -1)
							fSocketConnected[i] = false;
					}
				}
			}

			if (pDepthFrame != NULL)
				pDepthFrame->Release();
		}
	}

	for (int i=0; i<4; i++)
	{
		if (fSocketConnected[i])
			SocketHelper::CloseConnection(hSocket[i]);
	}
	fShouldExit = true;

	return 0;
}

bool LoadFromRegistry()
{
	bool bValue;
	std::string strValue;

	HKEY hKey;
	LONG lRes = RegOpenKeyEx(HKEY_CURRENT_USER, "SOFTWARE\\KinectTransport", 0, KEY_READ, &hKey);
	if (lRes != ERROR_SUCCESS)
		return false;

	for (int i=0; i<4; i++)
	{
		std::string keyName;

		keyName = "HostEnabled" + i;
		if (RegistryHelper::GetBoolRegValue(hKey, keyName.c_str(), bValue, true))
			fHostEnabled[i] = bValue;
		else
			return false;
		
		keyName = "DestinationHost" + i;
		if (RegistryHelper::GetStringRegValue(hKey, keyName.c_str(), strValue, "127.0.0.1"))
			strDestinationHost[i] = strValue;
		else
			return false;

		keyName = "SendSkeletonData" + i;
		if (RegistryHelper::GetBoolRegValue(hKey, keyName.c_str(), bValue, true))
			fSendBodiesData[i] = bValue;
		else
			return false;

		keyName = "SendDepthData" + i;
		if (RegistryHelper::GetBoolRegValue(hKey, keyName.c_str(), bValue, true))
			fSendDepthData[i] = bValue;
		else
			return false;
	}

	RegCloseKey(hKey);
	return true;
}

bool SaveToRegistry()
{
	HKEY hKey;
	LONG lRes = RegOpenKeyEx(HKEY_CURRENT_USER, "SOFTWARE\\KinectTransport", 0, KEY_READ | KEY_SET_VALUE, &hKey);
	if (lRes != ERROR_SUCCESS)
	{
		// no key, lets create one
		HKEY hSoftwareKey;
		lRes = RegOpenKeyEx(HKEY_CURRENT_USER, "SOFTWARE", 0, KEY_READ | KEY_SET_VALUE, &hSoftwareKey);
		if (lRes != ERROR_SUCCESS || !RegistryHelper::CreateRegistryKey(hSoftwareKey, "KinectTransport", hKey))
			return false;
	}

	// now save our values
	for (int i=0; i<4; i++)
	{
		std::string keyName;

		keyName = "HostEnabled" + i;
		DWORD dValue = fHostEnabled[i] ? 1 : 0;
		lRes = RegSetValueEx(hKey, keyName.c_str(), 0, REG_DWORD, (unsigned char*)&dValue, sizeof(DWORD));

		keyName = "DestinationHost" + i;
		lRes = RegSetValueEx(hKey, keyName.c_str(), 0, REG_SZ, (unsigned char*)strDestinationHost[i].c_str(), strDestinationHost[i].length() * sizeof(TCHAR));

		keyName = "SendSkeletonData" + i;
		dValue = fSendBodiesData[i] ? 1 : 0;
		lRes = RegSetValueEx(hKey, keyName.c_str(), 0, REG_DWORD, (unsigned char*)&dValue, sizeof(DWORD));

		keyName = "SendDepthData" + i;
		dValue = fSendDepthData[i] ? 1 : 0;
		lRes = RegSetValueEx(hKey, keyName.c_str(), 0, REG_DWORD, (unsigned char*)&dValue, sizeof(DWORD));
	}

	RegCloseKey(hKey);
	return true;
}

void SetupIcon(bool connected, bool modify)
{
	// Fill the NOTIFYICONDATA structure and call Shell_NotifyIcon

	// zero the structure - note:	Some Windows funtions require this but
	//								I can't be bothered which ones do and
	//								which ones don't.
	ZeroMemory(&niData,sizeof(NOTIFYICONDATA));

	// get Shell32 version number and set the size of the structure
	//		note:	the MSDN documentation about this is a little
	//				dubious and I'm not at all sure if the method
	//				bellow is correct
	ULONGLONG ullVersion = GetDllVersion(_T("Shell32.dll"));
	if(ullVersion >= MAKEDLLVERULL(5, 0,0,0))
		niData.cbSize = sizeof(NOTIFYICONDATA);
	else niData.cbSize = NOTIFYICONDATA_V2_SIZE;

	// the ID number can be anything you choose
	niData.uID = TRAYICONID;

	// state which structure members are valid
	niData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;

	// the window to send messages to and the message to send
	//		note:	the message value should be in the
	//				range of WM_APP through 0xBFFF
	niData.hWnd = hAppHwnd;
    niData.uCallbackMessage = SWM_TRAYMSG;

	// tooltip message
    lstrcpyn(niData.szTip, _T("Kinect Transport\nis running."), sizeof(niData.szTip)/sizeof(TCHAR));

	// setup the two icons
	niData.hIcon = (HICON)LoadImage(hAppInstance, MAKEINTRESOURCE(connected ? IDI_GREENLIGHT : IDI_REDLIGHT), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
	Shell_NotifyIcon(modify ? NIM_MODIFY : NIM_ADD, &niData);

	// free icon handle
	if(niData.hIcon && DestroyIcon(niData.hIcon))
		niData.hIcon = NULL;
}

//	Initialize the window and tray icon
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	// prepare for XP style controls
	InitCommonControls();

	 // store instance handle and create dialog
	hInst = hInstance;
	hAppHwnd = CreateDialog( hInstance, MAKEINTRESOURCE(IDD_DLG_DIALOG),
		NULL, (DLGPROC)DlgProc );
	if (!hAppHwnd) return FALSE;

	// setup icon
	SetupIcon(false, false);

	// get registry key for settings
	for (int i=0; i<4; i++)
	{
		strDestinationHost[i] = "127.0.0.1";
		strConnectedHost[i] = "";
		fSendBodiesData[i] = true;
		fSendDepthData[i] = false;
	}

	// try to load from registry, if we cannot try to save (in order to create key
	if (!LoadFromRegistry())
		SaveToRegistry();

	return TRUE;
}

BOOL OnInitDialog(HWND hWnd)
{
	HMENU hMenu = GetSystemMenu(hWnd,FALSE);
	if (hMenu)
	{
		AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
		AppendMenu(hMenu, MF_STRING, IDM_ABOUT, _T("About"));
	}
	HICON hIcon = (HICON)LoadImage(hInst,
		MAKEINTRESOURCE(IDI_GREENLIGHT),
		IMAGE_ICON, 0,0, LR_SHARED|LR_DEFAULTSIZE);
	SendMessage(hWnd,WM_SETICON,ICON_BIG,(LPARAM)hIcon);
	SendMessage(hWnd,WM_SETICON,ICON_SMALL,(LPARAM)hIcon);
	return TRUE;
}

// Name says it all
void ShowContextMenu(HWND hWnd)
{
	POINT pt;
	GetCursorPos(&pt);
	HMENU hMenu = CreatePopupMenu();
	if(hMenu)
	{
		InsertMenu(hMenu, -1, MF_BYPOSITION, SWM_SHOW, _T("Options"));
		InsertMenu(hMenu, -1, MF_BYPOSITION, SWM_EXIT, _T("Exit"));

		// note:	must set window to the foreground or the
		//			menu won't disappear when it should
		SetForegroundWindow(hWnd);
		TrackPopupMenu(hMenu, TPM_BOTTOMALIGN, pt.x, pt.y, 0, hWnd, NULL );
		DestroyMenu(hMenu);
	}
}

// Get dll version number
ULONGLONG GetDllVersion(LPCTSTR lpszDllName)
{
    ULONGLONG ullVersion = 0;
	HINSTANCE hinstDll;
    hinstDll = LoadLibrary(lpszDllName);
    if(hinstDll)
    {
        DLLGETVERSIONPROC pDllGetVersion;
        pDllGetVersion = (DLLGETVERSIONPROC)GetProcAddress(hinstDll, "DllGetVersion");
        if(pDllGetVersion)
        {
            DLLVERSIONINFO dvi;
            HRESULT hr;
            ZeroMemory(&dvi, sizeof(dvi));
            dvi.cbSize = sizeof(dvi);
            hr = (*pDllGetVersion)(&dvi);
            if(SUCCEEDED(hr))
				ullVersion = MAKEDLLVERULL(dvi.dwMajorVersion, dvi.dwMinorVersion,0,0);
        }
        FreeLibrary(hinstDll);
    }
    return ullVersion;
}

// Message handler for the app
INT_PTR CALLBACK DlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	int enabledControls[4] = { IDC_ENABLED1, IDC_ENABLED2, IDC_ENABLED3, IDC_ENABLED4 };
	int hostControls[4] = { IDC_HOST1, IDC_HOST2, IDC_HOST3, IDC_HOST4 };
	int bodiesEnabledControls[4] = { IDC_BODIESENABLED1, IDC_BODIESENABLED2, IDC_BODIESENABLED3, IDC_BODIESENABLED4 };
	int depthEnabledControls[4] = { IDC_DEPTHENABLED1, IDC_DEPTHENABLED2, IDC_DEPTHENABLED3, IDC_DEPTHENABLED4 };

	switch (message) 
	{
	case SWM_TRAYMSG:
		switch(lParam)
		{
		case WM_LBUTTONDBLCLK:
			ShowWindow(hWnd, SW_RESTORE);
			break;
		case WM_RBUTTONDOWN:
		case WM_CONTEXTMENU:
			ShowContextMenu(hWnd);
		}
		break;
	case WM_SYSCOMMAND:
		if((wParam & 0xFFF0) == SC_MINIMIZE)
		{
			ShowWindow(hWnd, SW_HIDE);
			return 1;
		}
		else if(wParam == IDM_ABOUT)
			DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
		break;
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam); 

		switch (wmEvent)
		{
			case BN_CLICKED:
				bool changed = false;
				for (int i=0; i<4; i++)
				{
					changed |= (wmId == enabledControls[i]);
					changed |= (wmId == bodiesEnabledControls[i]);
					changed |= (wmId == depthEnabledControls[i]);
				}
					
				if (changed)
				{
					// get values from window and write them to registry
					for (int i=0; i<4; i++)
					{
						char destinationHost[100];
						GetDlgItemText(hWnd, hostControls[i], destinationHost, 100);
						strDestinationHost[i] = destinationHost;
						fHostEnabled[i] = (IsDlgButtonChecked(hWnd, enabledControls[i]) == 1);
						fSendBodiesData[i] = (IsDlgButtonChecked(hWnd, bodiesEnabledControls[i]) == 1);
						fSendDepthData[i] = (IsDlgButtonChecked(hWnd, depthEnabledControls[i]) == 1);
					}
					SaveToRegistry();
				}
				break;
		}

		switch (wmId)
		{
		case SWM_SHOW:
			for (int i=0; i<4; i++)
			{
				SetDlgItemText(hWnd, hostControls[i], strDestinationHost[i].c_str());
				CheckDlgButton(hWnd, enabledControls[i], fHostEnabled[i] ? 1 : 0);
				CheckDlgButton(hWnd, bodiesEnabledControls[i], fSendBodiesData[i] ? 1 : 0);
				CheckDlgButton(hWnd, depthEnabledControls[i], fSendDepthData[i] ? 1 : 0);
			}
			ShowWindow(hWnd, SW_RESTORE);
			break;
		case IDOK:
			// get values from window and write them to registry
			for (int i=0; i<4; i++)
			{
				char destinationHost[100];
				GetDlgItemText(hWnd, hostControls[i], destinationHost, 100);
				strDestinationHost[i] = destinationHost;
				fHostEnabled[i] = (IsDlgButtonChecked(hWnd, enabledControls[i]) == 1);
				fSendBodiesData[i] = (IsDlgButtonChecked(hWnd, bodiesEnabledControls[i]) == 1);
				fSendDepthData[i] = (IsDlgButtonChecked(hWnd, depthEnabledControls[i]) == 1);
			}
			SaveToRegistry();
			break;
		case SWM_EXIT:
			fShouldDisconnectKinect = true;
			DestroyWindow(hWnd);
			break;
		case IDM_ABOUT:
			DialogBox(hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
			break;

		}
		return 1;
	case WM_INITDIALOG:
		return OnInitDialog(hWnd);
	case WM_CLOSE:
		ShowWindow(hWnd, SW_HIDE);
		break;
	case WM_DESTROY:
		niData.uFlags = 0;
		Shell_NotifyIcon(NIM_DELETE,&niData);
		PostQuitMessage(0);
		break;
	}
	return 0;
}

// Message handler for about box.
LRESULT CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		return TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) 
		{
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
		}
		break;
	}
	return FALSE;
}