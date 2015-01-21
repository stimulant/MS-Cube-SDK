#pragma once
#include "DeployApp.h"
#include <vector>
#include <map>
#include <string>

class DeployManager
{
	//std::vector<DeployApp*> mApps;
	std::map<std::string, DeployApp*> mApps;
	static DeployManager* mInstance;

public:
	DeployManager(void);
	~DeployManager(void);

	static DeployManager* instance();

	bool LoadFromRegistry();
	bool SaveToRegistry();

	std::map<std::string, DeployApp*>& GetApps() { return mApps; }
	void AddDeployApp(std::string appDirectory, std::string appExecutable);
	void ServerUpdate();
	bool SendAppListToClient(SOCKET clientSocket);
	bool SendToClient(SOCKET clientSocket);
	bool StartApp(SOCKET clientSocket, std::string appExecutable);
};

