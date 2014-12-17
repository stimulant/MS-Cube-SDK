#pragma once
#include "DeployFile.h"
#include <vector>
#include <string >

class DeployApp
{
	std::string mappDirectory;
	std::string mappExecutable;
	std::vector<DeployFile*> mfiles;

	bool AddDirectoryFiles(std::string rootDirectory, std::string directory);

public:
	DeployApp(std::string appDirectory, std::string appExecutable);
	~DeployApp(void);

	bool SendToClient(SOCKET clientSocket);
};

