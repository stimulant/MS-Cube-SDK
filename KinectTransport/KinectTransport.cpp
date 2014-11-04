#include "stdafx.h"
#include "resource.h"
#include <process.h>
#include <ctime>
#include "KinectData.h"

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
bool			fSocketConnected;
std::string		strDestinationHost;
std::string		strConnectedHost;
bool			fSendDepthData;
bool			fSendBodiesData;

// Kinect
KinectData *pKinectData;
unsigned int __stdcall KinectThread(void* data);

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

	fSocketConnected = false;
	fShouldExit = false;
	fShouldDisconnectKinect = false;

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
	
	return (int) msg.wParam;
}

void DebugOutput(LPCTSTR lpszFormat, ...)
{
    va_list args;
    va_start(args, lpszFormat);
    int nBuf;
    TCHAR szBuffer[512]; // get rid of this hard-coded buffer
    nBuf = _vsntprintf(szBuffer, 511, lpszFormat, args);
    ::OutputDebugString(szBuffer);
    va_end(args);
}

unsigned int __stdcall KinectThread(void* data)
{
	while (!fShouldDisconnectKinect)
	{
		// try to connect if we aren't connected
		if (!fSocketConnected || strDestinationHost != strConnectedHost)
		{
			fSocketConnected = pKinectData->ConnectToHost(3000, strDestinationHost.c_str());
			if (fSocketConnected)
			{
				SetupIcon(true);
				strConnectedHost = strDestinationHost;
			}
		}
		
		if (fSocketConnected)
		{
			fSocketConnected = pKinectData->UpdateKinect(fSendBodiesData, fSendDepthData);
			if (!fSocketConnected)
				SetupIcon(false);
		}
	}

	if (fSocketConnected)
		pKinectData->CloseConnection();
	fShouldExit = true;

	return 0;
}


bool GetBoolRegValue(HKEY hKey, const std::string &strValueName, bool &bValue, bool bDefaultValue)
{
    DWORD nDefValue((bDefaultValue) ? 1 : 0);
    DWORD nResult(nDefValue);
	DWORD dwBufferSize(sizeof(DWORD));
    LONG nError = ::RegQueryValueEx(hKey,
        strValueName.c_str(),
        0,
        NULL,
        reinterpret_cast<LPBYTE>(&nResult),
        &dwBufferSize);
    if (ERROR_SUCCESS == nError)
    {
        bValue = (nResult != 0);
    }
    return (ERROR_SUCCESS == nError);
}


LONG GetStringRegValue(HKEY hKey, const std::string &strValueName, std::string &strValue, const std::string &strDefaultValue)
{
    strValue = strDefaultValue;
    CHAR szBuffer[512];
    DWORD dwBufferSize = sizeof(szBuffer);
    ULONG nError;
    nError = RegQueryValueEx(hKey, strValueName.c_str(), 0, NULL, (LPBYTE)szBuffer, &dwBufferSize);
    if (ERROR_SUCCESS == nError)
    {
        strValue = szBuffer;
    }
    return (ERROR_SUCCESS == nError);
}

bool CreateRegistryKey(HKEY hKeyRoot, LPCTSTR pszSubKey, HKEY &hNewKey)
{
    DWORD dwFunc;
    LONG  lRet;
    SECURITY_DESCRIPTOR SD;
    SECURITY_ATTRIBUTES SA;

    if(!InitializeSecurityDescriptor(&SD, SECURITY_DESCRIPTOR_REVISION))
        return false;
    if(!SetSecurityDescriptorDacl(&SD, true, 0, false))
        return false;

    SA.nLength             = sizeof(SA);
    SA.lpSecurityDescriptor = &SD;
    SA.bInheritHandle      = false;
    lRet = RegCreateKeyEx(
        hKeyRoot,
        pszSubKey,
        0,
        (LPTSTR)NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_WRITE,
        &SA,
        &hNewKey,
        &dwFunc
    );

    if(lRet == ERROR_SUCCESS)
        return true;

    SetLastError((DWORD)lRet);
    return false;
}

bool LoadFromRegistry()
{
	bool bValue;
	std::string strValue;

	HKEY hKey;
	LONG lRes = RegOpenKeyEx(HKEY_CURRENT_USER, "SOFTWARE\\KinectTransport", 0, KEY_READ, &hKey);
	if (lRes != ERROR_SUCCESS)
		return false;

	if (GetStringRegValue(hKey, "DestinationHost", strValue, "127.0.0.1"))
		strDestinationHost = strValue;
	else
		return false;
	if (GetBoolRegValue(hKey, "SendSkeletonData", bValue, true))
		fSendBodiesData = bValue;
	else
		return false;
	if (GetBoolRegValue(hKey, "SendDepthData", bValue, true))
		fSendDepthData = bValue;
	else
		return false;

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
		if (lRes != ERROR_SUCCESS || !CreateRegistryKey(hSoftwareKey, "KinectTransport", hKey))
			return false;
	}

	// now save our values
	lRes = RegSetValueEx(hKey, "DestinationHost", 0, REG_SZ, (unsigned char*)strDestinationHost.c_str(), strDestinationHost.length() * sizeof(TCHAR));
	DWORD dValue = fSendBodiesData ? 1 : 0;
	lRes = RegSetValueEx(hKey, "SendSkeletonData", 0, REG_DWORD, (unsigned char*)&dValue, sizeof(DWORD));
	dValue = fSendDepthData ? 1 : 0;
	lRes = RegSetValueEx(hKey, "SendDepthData", 0, REG_DWORD, (unsigned char*)&dValue, sizeof(DWORD));
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
	strDestinationHost = "127.0.0.1";
	strConnectedHost = "";
	fSendBodiesData = true;
	fSendDepthData = false;

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
				if (wmId == IDC_SKELETONDATA || wmId == IDC_DEPTHDATA)
				{
					// get values from window and write them to registry
					char destinationHost[100];
					GetDlgItemText(hWnd, IDC_DESTINATIONHOST, destinationHost, 100);
					strDestinationHost = destinationHost;
					fSendBodiesData = (IsDlgButtonChecked(hWnd, IDC_SKELETONDATA) == 1);
					fSendDepthData = (IsDlgButtonChecked(hWnd, IDC_DEPTHDATA) == 1);
					SaveToRegistry();
				}
				break;
		}

		switch (wmId)
		{
		case SWM_SHOW:
			SetDlgItemText(hWnd, IDC_DESTINATIONHOST, strDestinationHost.c_str());
			CheckDlgButton(hWnd, IDC_SKELETONDATA, fSendBodiesData ? 1 : 0);
			CheckDlgButton(hWnd, IDC_DEPTHDATA, fSendDepthData ? 1 : 0);
			ShowWindow(hWnd, SW_RESTORE);
			break;
		case IDOK:
			// get values from window and write them to registry
			char destinationHost[100];
			GetDlgItemText(hWnd, IDC_DESTINATIONHOST, destinationHost, 100);
			strDestinationHost = destinationHost;
			fSendBodiesData = (IsDlgButtonChecked(hWnd, IDC_SKELETONDATA) == 1);
			fSendDepthData = (IsDlgButtonChecked(hWnd, IDC_DEPTHDATA) == 1);
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