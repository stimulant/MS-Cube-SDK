#include "stdafx.h"
#include "DeployManager.h"

DeployManager* DeployManager::mInstance;

DeployManager* DeployManager::instance()
{
    if( !mInstance ){
        mInstance = new DeployManager();
    }
    return mInstance;
}

DeployManager::DeployManager(void)
{
}

DeployManager::~DeployManager(void)
{
}

void DeployManager::AddDeployApp(std::string appDirectory, std::string appExecutable)
{
	mApps[appExecutable] = new DeployApp( appDirectory, appExecutable );
}

void DeployManager::ServerUpdate()
{
}

bool DeployManager::SendAppListToClient(SOCKET clientSocket)
{
	// SEND COMMAND
	char command[32] = "LISTAPPS";
	send(clientSocket, command, 32, 0);
	char rec[32] = ""; 
	if (recv(clientSocket, rec, 2, 0) <= 0)
		return false;

	// send app count
	char appCountStr[10]; _ltoa((long)mApps.size(), appCountStr, 10);
	send(clientSocket, appCountStr, 10, 0);
	//DBOUT("SERVER: sent filesize: " << filesizeStr << "\n");
	if (recv(clientSocket, rec, 2, 0) <= 0)
		return false;
	//DBOUT("SERVER: received filesize ack\n");

	for(std::map<std::string, DeployApp*>::iterator iterator = mApps.begin(); iterator != mApps.end(); iterator++) 
	{
		// send app name
		send(clientSocket, iterator->first.c_str(), 256, 0);
		//DBOUT("SERVER: sent filename: " << iterator->first.c_str() << "\n");
		if (recv(clientSocket, rec, 2, 0) <= 0)
			return false;
		//DBOUT("SERVER: received filename ack\n");
	}
	return true;
}

bool DeployManager::SendToClient(SOCKET clientSocket)
{
	for(std::map<std::string, DeployApp*>::iterator iterator = mApps.begin(); iterator != mApps.end(); iterator++) 
	{
		if (!iterator->second->SendToClient(clientSocket))
			return false;
	}
	return true;
}

bool DeployManager::StartApp(SOCKET clientSocket, std::string appExecutable)
{
	// send command
	char command[32] = "STARTAPP";
	send(clientSocket, command, 32, 0);
	char rec[32] = ""; 
	if (recv(clientSocket, rec, 2, 0) <= 0)
		return false;

	// send executable name
	char executableName[128] = "";
	strcpy(executableName, appExecutable.c_str());
	executableName[appExecutable.length()] = '\0';
	send(clientSocket, executableName, 128, 0);
	if (recv(clientSocket, rec, 2, 0) <= 0)
		return false;

	return true;
}