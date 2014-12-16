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
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_bInit = false;
	m_pClientTab=NULL;
	m_pServerTab=NULL;

	m_nidIconData.cbSize			= sizeof(NOTIFYICONDATA);
	m_nidIconData.hWnd				= 0;
	m_nidIconData.uID				= 1;
	m_nidIconData.uCallbackMessage	= WM_TRAY_ICON_NOTIFY_MESSAGE;
	m_nidIconData.hIcon				= 0;
	m_nidIconData.szTip[0]			= 0;	
	m_nidIconData.uFlags			= NIF_MESSAGE;
	m_bTrayIconVisible				= FALSE;
	m_nDefaultMenuItem				= 0;
	m_bMinimizeToTray				= TRUE;

	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDeployToolDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB, m_ctrlTAB);
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
	
	m_nidIconData.hWnd = this->m_hWnd;
	m_nidIconData.uID = 1;
	
	return 0;
}

void CDeployToolDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	if(m_nidIconData.hWnd && m_nidIconData.uID>0 && TrayIsVisible())
	{
		Shell_NotifyIcon(NIM_DELETE,&m_nidIconData);
	}
}

BOOL CDeployToolDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	m_pClientTab = new CTabPageClient();
	m_pClientTab->Create(IDD_DIALOG_PAGE1,m_ctrlTAB.GetWindow(IDD_DIALOG_PAGE1));
	m_pServerTab = new CTabPageServer();
	m_pServerTab->Create(IDD_DIALOG_PAGE2,m_ctrlTAB.GetWindow(IDD_DIALOG_PAGE2));
	m_ctrlTAB.AddTabPane("Client", m_pClientTab);
	m_ctrlTAB.AddTabPane("Server", m_pServerTab);

	// create DeployManager and create test app
	DeployManager::instance()->AddDeployApp(
		"C:\\Users\\joel\\Desktop\\12_9_2014\\",
		"render_test.exe" );

	// start winsock
	SocketHelper::StartWinsock();

	// start up client
	m_pClientTab->Startup();
	
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
		dc.DrawIcon(x, y, m_hIcon);
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
	return static_cast<HCURSOR>(m_hIcon);
}

void CDeployToolDlg::OnMove(int x, int y)
{
	m_ctrlTAB.OnMove(x,y); 
}

void CDeployToolDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	CDialog::OnShowWindow(bShow, nStatus);
	m_ctrlTAB.SetDefaultPane(0);
}

BOOL CDeployToolDlg::TrayIsVisible()
{
	return m_bTrayIconVisible;
}

void CDeployToolDlg::TraySetIcon(HICON hIcon)
{
	ASSERT(hIcon);

	m_nidIconData.hIcon = hIcon;
	m_nidIconData.uFlags |= NIF_ICON;
}

void CDeployToolDlg::TraySetIcon(UINT nResourceID)
{
	ASSERT(nResourceID>0);
	HICON hIcon = 0;
	hIcon = AfxGetApp()->LoadIcon(nResourceID);
	if(hIcon)
	{
		m_nidIconData.hIcon = hIcon;
		m_nidIconData.uFlags |= NIF_ICON;
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
		m_nidIconData.hIcon = hIcon;
		m_nidIconData.uFlags |= NIF_ICON;
	}
	else
	{
		TRACE0("FAILED TO LOAD ICON\n");
	}
}

void CDeployToolDlg::TraySetToolTip(LPCTSTR lpszToolTip)
{
	ASSERT(strlen(lpszToolTip) > 0 && strlen(lpszToolTip) < 64);

	strcpy(m_nidIconData.szTip,lpszToolTip);
	m_nidIconData.uFlags |= NIF_TIP;
}

BOOL CDeployToolDlg::TrayShow()
{
	BOOL bSuccess = FALSE;
	if(!m_bTrayIconVisible)
	{
		bSuccess = Shell_NotifyIcon(NIM_ADD,&m_nidIconData);
		if(bSuccess)
			m_bTrayIconVisible= TRUE;
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
	if(m_bTrayIconVisible)
	{
		bSuccess = Shell_NotifyIcon(NIM_DELETE,&m_nidIconData);
		if(bSuccess)
			m_bTrayIconVisible= FALSE;
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
	if(m_bTrayIconVisible)
	{
		bSuccess = Shell_NotifyIcon(NIM_MODIFY,&m_nidIconData);
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
	bSuccess = m_mnuTrayMenu.LoadMenu(nResourceID);
	return bSuccess;
}

BOOL CDeployToolDlg::TraySetMenu(LPCTSTR lpszMenuName,UINT nDefaultPos)
{
	BOOL bSuccess;
	bSuccess = m_mnuTrayMenu.LoadMenu(lpszMenuName);
	return bSuccess;
}

BOOL CDeployToolDlg::TraySetMenu(HMENU hMenu,UINT nDefaultPos)
{
	m_mnuTrayMenu.Attach(hMenu);
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
			m_mnuTrayMenu.GetSubMenu(0)->TrackPopupMenu(TPM_BOTTOMALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON, pt.x, pt.y, this);
			m_mnuTrayMenu.GetSubMenu(0)->SetDefaultItem(m_nDefaultMenuItem,TRUE);
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
	if(m_bMinimizeToTray)
	{
		if ((nID & 0xFFF0) == SC_MINIMIZE)
		{
			if( TrayShow() )
			{
				this->ShowWindow(SW_HIDE);
				m_pClientTab->ShowWindow(SW_HIDE);
				m_pServerTab->ShowWindow(SW_HIDE);
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
	m_bMinimizeToTray = bMinimizeToTray;
}

void CDeployToolDlg::OnTcnSelchangeTab(NMHDR *pNMHDR, LRESULT *pResult)
{
	int index = TabCtrl_GetCurSel( m_ctrlTAB.m_hWnd );

	switch (index) 
	{
		case 0:
			// shutdown server
			m_pServerTab->Shutdown();
			// setup client
			m_pClientTab->Startup();
			break;
		case 1:	
			// shutdown client
			m_pClientTab->Shutdown();
			// setup server
			m_pServerTab->Startup();
			break;
	}
}
