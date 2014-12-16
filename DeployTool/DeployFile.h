#pragma once
#include <string>

class DeployFile
{
	std::string m_strFileName;
	std::string m_strPath;
	WIN32_FIND_DATA m_FindData;
	LONGLONG m_FileSize;

public:
	DeployFile(std::string strFileName, std::string strPath, WIN32_FIND_DATA findData);
	~DeployFile(void);
	bool SendToClient(SOCKET hSocket);
	static bool ReceiveFile(SOCKET hSocket);
};

