#pragma once
#include <string>

// CTabPageClient dialog

class CTabPageClient : public CDialog
{
	DECLARE_DYNAMIC(CTabPageClient)

	bool mfExitThread;
	SOCKET mSocket;
	bool mConnected;
	std::string mStrDeployServer;
	int mDeployPort;
	char* mRecvBuffer;

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
