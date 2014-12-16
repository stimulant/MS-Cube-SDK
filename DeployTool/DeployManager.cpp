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