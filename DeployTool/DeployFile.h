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
	bool SendToClient(std::string rootDirector, SOCKET hSocket);
	bool AskIfNeedsUpdate(std::string rootDirector, SOCKET hSocket);
	static bool DoesFileNeedUpdate(SOCKET hSocket);
	static bool ReceiveFile(SOCKET hSocket);
};

