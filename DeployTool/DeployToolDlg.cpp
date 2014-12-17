// TabDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DeployTool.h"
#include "DeployToolDlg.h"

#include "DeployManager.h"
#include "SocketHelper.h"
#include "RegistryHelper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CDeployToolDlg dialog

CDeployToolDlg::CDeployToolDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CDeployToolDlg::IDD, pParent)
{
	mhIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	mbInit = false;
	mpClientTab=NULL;
	mpServerTab=NULL;

	mnidIconData.cbSize			= sizeof(NOTIFYICONDATA);
	mnidIconData.hWnd				= 0;
	mnidIconData.uID				= 1;
	mnidIconData.uCallbackMessage	= WM_TRAY_ICON_NOTIFY_MESSAGE;
	mnidIconData.hIcon				= 0;
	mnidIconData.szTip[0]			= 0;	
	mnidIconData.uFlags			= NIF_MESSAGE;
	mbTrayIconVisible				= FALSE;
	mnDefaultMenuItem				= 0;
	mbMinimizeToTray				= TRUE;

	mhIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDeployToolDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB, mctrlTAB);
}

BEGIN_MESSAGE_MAP(CDeployToolDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SYSCOMMAND()
	ON_WM_MOVE()
	ON_WM_SHOWWINDOW()
	ON_MESSAGE(WM_TRAY_ICON_NOTIFY_MESSAGE,OnTrayNotify)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB, &CDeployToolDlg::OnTcnSelchangeTab)
END_MESSAGE_MAP()


// CDeployToolDlg message handlers

int CDeployToolDlg::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	mnidIconData.hWnd = this->m_hWnd;
	mnidIconData.uID = 1;
	
	return 0;
}

void CDeployToolDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	if(mnidIconData.hWnd && mnidIconData.uID>0 && TrayIsVisible())
	{
		Shell_NotifyIcon(NIM_DELETE,&mnidIconData);
	}
}

BOOL CDeployToolDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(mhIcon, TRUE);			// Set big icon
	SetIcon(mhIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	mpClientTab = new CTabPageClient();
	mpClientTab->Create(IDD_DIALOG_PAGE1,mctrlTAB.GetWindow(IDD_DIALOG_PAGE1));
	mpServerTab = new CTabPageServer();
	mpServerTab->Create(IDD_DIALOG_PAGE2,mctrlTAB.GetWindow(IDD_DIALOG_PAGE2));
	mctrlTAB.AddTabPane("Client", mpClientTab);
	mctrlTAB.AddTabPane("Server", mpServerTab);

	// start winsock
	SocketHelper::StartWinsock();

	// start up client
	mpClientTab->Startup();
	
	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	TraySetIcon(IDR_MAINFRAME);
	TraySetToolTip("ToolTip for tray icon");
	TraySetMenu(IDR_MENU1);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CDeployToolDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, mhIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CDeployToolDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(mhIcon);
}

void CDeployToolDlg::OnMove(int x, int y)
{
	mctrlTAB.OnMove(x,y); 
}

void CDeployToolDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);
	mctrlTAB.SetDefaultPane(0);
}

BOOL CDeployToolDlg::TrayIsVisible()
{
	return mbTrayIconVisible;
}

void CDeployToolDlg::TraySetIcon(HICON hIcon)
{
	ASSERT(hIcon);

	mnidIconData.hIcon = hIcon;
	mnidIconData.uFlags |= NIF_ICON;
}

void CDeployToolDlg::TraySetIcon(UINT nResourceID)
{
	ASSERT(nResourceID>0);
	HICON hIcon = 0;
	hIcon = AfxGetApp()->LoadIcon(nResourceID);
	if(hIcon)
	{
		mnidIconData.hIcon = hIcon;
		mnidIconData.uFlags |= NIF_ICON;
	}
	else
	{
		TRACE0("FAILED TO LOAD ICON\n");
	}
}

void CDeployToolDlg::TraySetIcon(LPCTSTR lpszResourceName)
{
	HICON hIcon = 0;
	hIcon = AfxGetApp()->LoadIcon(lpszResourceName);
	if(hIcon)
	{
		mnidIconData.hIcon = hIcon;
		mnidIconData.uFlags |= NIF_ICON;
	}
	else
	{
		TRACE0("FAILED TO LOAD ICON\n");
	}
}

void CDeployToolDlg::TraySetToolTip(LPCTSTR lpszToolTip)
{
	ASSERT(strlen(lpszToolTip) > 0 && strlen(lpszToolTip) < 64);

	strcpy(mnidIconData.szTip,lpszToolTip);
	mnidIconData.uFlags |= NIF_TIP;
}

BOOL CDeployToolDlg::TrayShow()
{
	BOOL bSuccess = FALSE;
	if(!mbTrayIconVisible)
	{
		bSuccess = Shell_NotifyIcon(NIM_ADD,&mnidIconData);
		if(bSuccess)
			mbTrayIconVisible= TRUE;
	}
	else
	{
		TRACE0("ICON ALREADY VISIBLE");
	}
	return bSuccess;
}

BOOL CDeployToolDlg::TrayHide()
{
	BOOL bSuccess = FALSE;
	if(mbTrayIconVisible)
	{
		bSuccess = Shell_NotifyIcon(NIM_DELETE,&mnidIconData);
		if(bSuccess)
			mbTrayIconVisible= FALSE;
	}
	else
	{
		TRACE0("ICON ALREADY HIDDEN");
	}
	return bSuccess;
}

BOOL CDeployToolDlg::TrayUpdate()
{
	BOOL bSuccess = FALSE;
	if(mbTrayIconVisible)
	{
		bSuccess = Shell_NotifyIcon(NIM_MODIFY,&mnidIconData);
	}
	else
	{
		TRACE0("ICON NOT VISIBLE");
	}
	return bSuccess;
}

BOOL CDeployToolDlg::TraySetMenu(UINT nResourceID,UINT nDefaultPos)
{
	BOOL bSuccess;
	bSuccess = mmnuTrayMenu.LoadMenu(nResourceID);
	return bSuccess;
}

BOOL CDeployToolDlg::TraySetMenu(LPCTSTR lpszMenuName,UINT nDefaultPos)
{
	BOOL bSuccess;
	bSuccess = mmnuTrayMenu.LoadMenu(lpszMenuName);
	return bSuccess;
}

BOOL CDeployToolDlg::TraySetMenu(HMENU hMenu,UINT nDefaultPos)
{
	mmnuTrayMenu.Attach(hMenu);
	return TRUE;
}

LRESULT CDeployToolDlg::OnTrayNotify(WPARAM wParam, LPARAM lParam) 
{ 
    UINT uID; 
    UINT uMsg; 
 
    uID = (UINT) wParam; 
    uMsg = (UINT) lParam; 
 
	if (uID != 1)
		return 0;
	
	CPoint pt;	

    switch (uMsg ) 
	{ 
		case WM_MOUSEMOVE:
			break;
		case WM_LBUTTONDOWN:
			break;
		case WM_LBUTTONDBLCLK:
			break;
	
		case WM_RBUTTONDOWN:
		case WM_CONTEXTMENU:
			GetCursorPos(&pt);

			// show menu
			mmnuTrayMenu.GetSubMenu(0)->TrackPopupMenu(TPM_BOTTOMALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON, pt.x, pt.y, this);
			mmnuTrayMenu.GetSubMenu(0)->SetDefaultItem(mnDefaultMenuItem,TRUE);
			break;
		case WM_RBUTTONDBLCLK:
			//GetCursorPos(&pt);
			//ClientToScreen(&pt);
			//OnTrayRButtonDblClk(pt);
			break;
    } 
     return 0; 
 } 

void CDeployToolDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if(mbMinimizeToTray)
	{
		if ((nID & 0xFFF0) == SC_MINIMIZE)
		{
			if( TrayShow() )
			{
				this->ShowWindow(SW_HIDE);
				mpClientTab->ShowWindow(SW_HIDE);
				mpServerTab->ShowWindow(SW_HIDE);
			}
		}
		else
			CDialog::OnSysCommand(nID, lParam);	
	}
	else
		CDialog::OnSysCommand(nID, lParam);
}

void CDeployToolDlg::TraySetMinimizeToTray(BOOL bMinimizeToTray)
{
	mbMinimizeToTray = bMinimizeToTray;
}

void CDeployToolDlg::OnTcnSelchangeTab(NMHDR *pNMHDR, LRESULT *pResult)
{
	int index = TabCtrl_GetCurSel( mctrlTAB.m_hWnd );

	switch (index) 
	{
		case 0:
			// shutdown server
			mpServerTab->Shutdown();
			// setup client
			mpClientTab->Startup();
			break;
		case 1:	
			// shutdown client
			mpClientTab->Shutdown();
			// setup server
			mpServerTab->Startup();
			break;
	}
}
