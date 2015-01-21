#pragma once
#include "DeployFile.h"
#include <vector>
#include <string >

class DeployApp
{
	std::string mAppDirectory;
	std::string mAppExecutable;
	std::vector<DeployFile*> mfiles;

	bool AddDirectoryFiles(std::string rootDirectory, std::string directory);

public:
	DeployApp(std::string appDirectory, std::string appExecutable);
	~DeployApp(void);

	bool IsAppSelected(SOCKET clientSocket);
	bool Update(SOCKET clientSocket);
	bool SendToClient(SOCKET clientSocket);
	std::string GetAppDirectory() const { return mAppDirectory; }
	std::string GetAppExecutable() const { return mAppExecutable; }
};

