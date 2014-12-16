#pragma once
#include "DeployFile.h"
#include <vector>
#include <string >

class DeployApp
{
	std::string m_appDirectory;
	std::string m_appExecutable;
	std::vector<DeployFile*> m_files;

	bool AddDirectoryFiles(std::string directory);

public:
	DeployApp(std::string appDirectory, std::string appExecutable);
	~DeployApp(void);
};

