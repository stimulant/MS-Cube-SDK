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
	m_Apps.push_back( new DeployApp( appDirectory, appExecutable ) );
}

void DeployManager::ServerUpdate()
{
}

bool DeployManager::SendToClient(SOCKET clientSocket)
{
	for (unsigned int i=0; i<m_Apps.size(); i++)
	{
		if (!m_Apps[i]->SendToClient(clientSocket))
			return false;
	}
	return true;
}