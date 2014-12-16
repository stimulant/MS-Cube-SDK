#pragma once
#include <string>

// CTabPageClient dialog

class CTabPageClient : public CDialog
{
	DECLARE_DYNAMIC(CTabPageClient)

	bool m_fExitThread;
	SOCKET m_hSocket;
	bool m_fConnected;
	std::string m_strDeployServer;
	int m_DeployPort;
	char* m_RecvBuffer;

	void ClientThread();

public:
	CTabPageClient(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTabPageClient();

// Dialog Data
	enum { IDD = IDD_DIALOG_PAGE1 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);

	void Startup();
	void Shutdown();
};
