#pragma once
#include <string>
#include <vector>

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
	std::vector<std::string> mAppNames;

	void ClientThread();

public:
	CTabPageClient(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTabPageClient();
	BOOL OnInitDialog();

// Dialog Data
	enum { IDD = IDD_DIALOG_PAGE1 };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnChangeServerEdit();

	void Startup();
	void Shutdown();
};
