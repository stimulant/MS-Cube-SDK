#pragma once
#include "DeployFile.h"
#include <vector>
#include <string >

class DeployApp
{
	std::string mAppName;
	std::string mAppDirectory;
	std::string mAppExecutable;
	std::vector<DeployFile*> mFiles;

	bool AddDirectoryFiles(std::string rootDirectory, std::string directory);

public:
	DeployApp(std::string appName, std::string appDirectory, std::string appExecutable);
	~DeployApp(void);

	bool ServerIsAppSelected(SOCKET clientSocket);
	bool ServerUpdate(SOCKET clientSocket);
	bool ServerKillApp(SOCKET clientSocket);
	bool ServerStartApp(SOCKET clientSocket);
	std::string GetAppName() const { return mAppName; }
	std::string GetAppDirectory() const { return mAppDirectory; }
	std::string GetAppExecutable() const { return mAppExecutable; }
};

