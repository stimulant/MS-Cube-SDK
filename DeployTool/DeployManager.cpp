#include "stdafx.h"
#include "DeployManager.h"
#include "RegistryHelper.h"

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

bool DeployManager::LoadFromRegistry()
{
	std::string keyName;
	std::string strValue;

	HKEY hKey;
	LONG lRes = RegOpenKeyEx(HKEY_CURRENT_USER, "SOFTWARE\\DeployTool", 0, KEY_READ, &hKey);
	if (lRes != ERROR_SUCCESS)
		return false;

	// get number of apps
	int appCount;
	if (!RegistryHelper::GetIntRegValue(hKey, "AppCount", appCount, 0))
		return false;

	for (int i=0; i<appCount; i++)
	{
		std::string appDirectory, appExecutable;
		keyName = "AppDirectory" + i;
		if (!RegistryHelper::GetStringRegValue(hKey, keyName.c_str(), appDirectory, ""))
			return false;
		keyName = "AppExecutable" + i;
		if (!RegistryHelper::GetStringRegValue(hKey, keyName.c_str(), appExecutable, ""))
			return false;
		
		AddDeployApp(appDirectory, appExecutable);
	}

	RegCloseKey(hKey);

	// check to see if startup key exists
	lRes = RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_READ, &hKey);
	if (lRes != ERROR_SUCCESS)
		return false;
	bool startAppOnStartup = RegistryHelper::GetStringRegValue(hKey, "DeployTool", strValue, "");
	RegCloseKey(hKey);
	
	return true;
}

bool DeployManager::SaveToRegistry()
{
	HKEY hKey;
	LONG lRes = RegOpenKeyEx(HKEY_CURRENT_USER, "SOFTWARE\\DeployTool", 0, KEY_READ | KEY_SET_VALUE, &hKey);
	if (lRes != ERROR_SUCCESS)
	{
		// no key, lets create one
		HKEY hSoftwareKey;
		lRes = RegOpenKeyEx(HKEY_CURRENT_USER, "SOFTWARE", 0, KEY_READ | KEY_SET_VALUE, &hSoftwareKey);
		if (lRes != ERROR_SUCCESS || !RegistryHelper::CreateRegistryKey(hSoftwareKey, "DeployTool", hKey))
			return false;
	}

	// save app count
	int appCount = mApps.size();
	lRes = RegSetValueEx(hKey, "AppCount", 0, REG_DWORD, (BYTE *)&appCount, sizeof(appCount));

	// set app properties
	int i = 0;
	for(std::map<std::string, DeployApp*>::iterator iterator = mApps.begin(); iterator != mApps.end(); iterator++) 
	{
		std::string keyName;

		keyName = "AppDirectory" + i;
		lRes = RegSetValueEx(hKey, keyName.c_str(), 0, REG_SZ, (unsigned char*)iterator->second->GetAppDirectory().c_str(), iterator->second->GetAppDirectory().length() * sizeof(TCHAR));

		keyName = "AppExecutable" + i;
		lRes = RegSetValueEx(hKey, keyName.c_str(), 0, REG_SZ, (unsigned char*)iterator->second->GetAppExecutable().c_str(), iterator->second->GetAppExecutable().length() * sizeof(TCHAR));
		i++;
	}

	RegCloseKey(hKey);

	// check to see if startup key exists
	lRes = RegOpenKeyEx(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_READ | KEY_SET_VALUE, &hKey);
	if (lRes != ERROR_SUCCESS)
		return false;
	if (false)
	{
		char myPath[512];
		GetModuleFileName(NULL, myPath, 512);
		std::string strApplicationPath = myPath;
		lRes = RegSetValueEx(hKey, "DeployTool", 0, REG_SZ, (unsigned char*)strApplicationPath.c_str(), strApplicationPath.length() * sizeof(TCHAR));
	}
	else
		lRes = RegDeleteValue(hKey, "DeployTool");
	RegCloseKey(hKey);

	return true;
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