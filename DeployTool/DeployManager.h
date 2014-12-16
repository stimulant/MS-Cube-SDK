#pragma once
#include "DeployApp.h"
#include <vector>
#include <string>

class DeployManager
{
	std::vector<DeployApp*> m_Apps;
	static DeployManager* mInstance;

public:
	DeployManager(void);
	~DeployManager(void);

	static DeployManager* instance();
	void AddDeployApp(std::string appDirectory, std::string appExecutable);
	void ServerUpdate();
};

