#pragma once
#include <vector>

// CTabPageServer dialog

class CTabPageServer : public CDialog
{
	DECLARE_DYNAMIC(CTabPageServer)

	bool m_fExitThread;
	SOCKET m_hServerSocket;
	std::vector<SOCKET> m_hClients;
	int m_DeployPort;
	char* m_SendBuffer;
	char* m_RecvBuffer;

	void ServerConnectThread();
	void ServerUpdateThread();

public:
	CTabPageServer(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTabPageServer();

// Dialog Data
	enum { IDD = IDD_DIALOG_PAGE2 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);

	void Startup();
	void Shutdown();
};
