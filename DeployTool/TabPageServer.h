#pragma once
#include <vector>

// CTabPageServer dialog

class CTabPageServer : public CDialog
{
	DECLARE_DYNAMIC(CTabPageServer)

	bool mfExitThread;
	SOCKET mServerSocket;
	std::vector<SOCKET> mClients;
	int mDeployPort;
	char* mSendBuffer;
	char* mRecvBuffer;
	bool mAppListChanged;

	void ServerConnectThread();
	void ServerUpdateThread();

public:
	CTabPageServer(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTabPageServer();
	BOOL OnInitDialog();

// Dialog Data
	enum { IDD = IDD_DIALOG_PAGE2 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedAddApp();
	afx_msg void OnBnClickedRemoveApp();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);

	void Startup();
	void Shutdown();
};
