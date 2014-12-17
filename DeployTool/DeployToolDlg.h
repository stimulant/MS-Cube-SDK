// TabDlg.h : header file
//

#pragma once
#include "afxcmn.h"
#include "TabPageClient.h"
#include "TabPageServer.h"
#include "ibtabctrl.h"

#define WM_TRAY_ICON_NOTIFY_MESSAGE (WM_USER + 1)

// CDeployToolDlg dialog
class CDeployToolDlg : public CDialog
{
// Construction
public:
	CDeployToolDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_TAB_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON mhIcon;

	// Generated message map functions
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);	
	DECLARE_MESSAGE_MAP()

private:
	BOOL			mbMinimizeToTray;
	BOOL			mbTrayIconVisible;
	NOTIFYICONDATA	mnidIconData;
	CMenu			mmnuTrayMenu;
	UINT			mnDefaultMenuItem;

	CTabPageClient *mpClientTab;
	CTabPageServer *mpServerTab;
	int mbInit;

	afx_msg LRESULT OnTrayNotify(WPARAM wParam, LPARAM lParam);

public:
	CibTabCtrl mctrlTAB;
	afx_msg void OnMove(int x, int y);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);

	void TraySetMinimizeToTray(BOOL bMinimizeToTray = TRUE);
	BOOL TraySetMenu(UINT nResourceID,UINT nDefaultPos=0);	
	BOOL TraySetMenu(HMENU hMenu,UINT nDefaultPos=0);	
	BOOL TraySetMenu(LPCTSTR lpszMenuName,UINT nDefaultPos=0);	
	BOOL TrayUpdate();
	BOOL TrayShow();
	BOOL TrayHide();
	void TraySetToolTip(LPCTSTR lpszToolTip);
	void TraySetIcon(HICON hIcon);
	void TraySetIcon(UINT nResourceID);
	void TraySetIcon(LPCTSTR lpszResourceName);

	BOOL TrayIsVisible();
	afx_msg void OnTcnSelchangeTab(NMHDR *pNMHDR, LRESULT *pResult);
};
