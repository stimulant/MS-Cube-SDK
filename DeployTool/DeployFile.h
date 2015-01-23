#pragma once
#include <string>

class DeployFile
{
	std::string mStrFileName;
	std::string mStrPath;
	WIN32_FIND_DATA mFindData;
	LONGLONG mFileSize;

public:
	DeployFile(std::string strFileName, std::string strPath, WIN32_FIND_DATA findData);
	~DeployFile(void);
	bool ServerSendToClient(std::string appName, std::string rootDirector, SOCKET hSocket);
	bool ServerAskIfNeedsUpdate(std::string appName, std::string rootDirector, SOCKET hSocket, bool& doesNeedUpdate);
	bool ServerStartApp(SOCKET clientSocket);
	static bool ClientDoesFileNeedUpdate(SOCKET hSocket);
	static bool ClientReceiveFile(SOCKET hSocket);
};

