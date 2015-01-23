#include "stdafx.h"
#include "DeployApp.h"
#include <windows.h>

DeployApp::DeployApp(std::string appName, std::string appDirectory, std::string appExecutable)
{
	mAppName = appName;
	mAppDirectory = appDirectory;
	mAppExecutable = appExecutable;
}

DeployApp::~DeployApp(void)
{
}

bool DeployApp::AddDirectoryFiles(std::string rootDirectory, std::string directory)
{
	// go through directory and add files
	WIN32_FIND_DATA ffd;
	LARGE_INTEGER filesize;
   
   // List all the files in the directory with some info about them.
	std::string directoryName = rootDirectory + "\\" + directory + "\\*";
	if (directory == "")
		directoryName = rootDirectory + "\\*";
	HANDLE hFind = FindFirstFile(directoryName.c_str(), &ffd);
	if (INVALID_HANDLE_VALUE == hFind) 
		return false;
	do
	{
		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (strcmp(ffd.cFileName, ".") != 0 && strcmp(ffd.cFileName, "..") != 0)
			{
				_tprintf(TEXT("  %s   <DIR>\n"), ffd.cFileName);

				// iterate over directory files
				std::string localDirectoryName = directory + "\\" + ffd.cFileName;
				if (directory == "")
					localDirectoryName = ffd.cFileName;
				AddDirectoryFiles(rootDirectory, localDirectoryName);
			}
		}
		else
		{
			filesize.LowPart = ffd.nFileSizeLow;
			filesize.HighPart = ffd.nFileSizeHigh;
			_tprintf(TEXT("  %s   %ld bytes\n"), ffd.cFileName, filesize.QuadPart);

			// we have a valid file, add it
			std::string fileName = ffd.cFileName;
			mFiles.push_back(new DeployFile(fileName, directory, ffd));
		}
	}
	while (FindNextFile(hFind, &ffd) != 0);
	FindClose(hFind);

	return true;
}

bool DeployApp::ServerIsAppSelected(SOCKET clientSocket)
{
	// send command
	char command[32] = "ISAPPSELECTED";
	send(clientSocket, command, 32, 0);
	char rec[32] = ""; 
	if (recv(clientSocket, rec, 2, 0) <= 0)
		return false;

	// send executable name
	char appName[MAX_PATH] = "";
	strcpy(appName, mAppName.c_str());
	appName[mAppName.length()] = '\0';
	send(clientSocket, appName, MAX_PATH, 0);
	if (recv(clientSocket, rec, 2, 0) <= 0)
		return false;
	
	return (strcmp(rec, "YS") == 0);
}

bool DeployApp::ServerUpdate(SOCKET clientSocket)
{
	// do this every time to make sure we have a fresh list
	mFiles.clear();
	AddDirectoryFiles(mAppDirectory, "");

	bool appKilled = false;
	bool appUpdated = false;
	for (unsigned int i=0; i<mFiles.size(); i++)
	{
		// check if the app needs an update of this file
		bool doesNeedUpdate = false;
		if (!mFiles[i]->ServerAskIfNeedsUpdate(mAppName, mAppDirectory, clientSocket, doesNeedUpdate))
			continue;

		// send the file if so
		if (doesNeedUpdate)
		{
			if (!appKilled)
			{
				ServerKillApp(clientSocket);
				appKilled = true;
			}
			appUpdated = true;
			if (!mFiles[i]->ServerSendToClient(mAppName, mAppDirectory, clientSocket))
				return false;
		}
		Sleep(100);
	}

	// then start it up!
	if (appUpdated)
		ServerStartApp(clientSocket);

	return true;
}

bool DeployApp::ServerKillApp(SOCKET clientSocket)
{
	// send command
	char command[32] = "KILLAPP";
	send(clientSocket, command, 32, 0);
	char rec[32] = ""; 
	if (recv(clientSocket, rec, 2, 0) <= 0)
		return false;

	// send app name
	send(clientSocket, mAppName.c_str(), mAppName.length(), 0);
	if (recv(clientSocket, rec, 2, 0) <= 0)
		return false;

	// send executable name
	send(clientSocket, mAppExecutable.c_str(), mAppExecutable.length(), 0);
	if (recv(clientSocket, rec, 2, 0) <= 0)
		return false;

	return true;
}

bool DeployApp::ServerStartApp(SOCKET clientSocket)
{
	// send command
	char command[32] = "STARTAPP";
	send(clientSocket, command, 32, 0);
	char rec[32] = ""; 
	if (recv(clientSocket, rec, 2, 0) <= 0)
		return false;

	// send app name
	send(clientSocket, mAppName.c_str(), mAppName.length(), 0);
	if (recv(clientSocket, rec, 2, 0) <= 0)
		return false;

	// send executable name
	send(clientSocket, mAppExecutable.c_str(), mAppExecutable.length(), 0);
	if (recv(clientSocket, rec, 2, 0) <= 0)
		return false;

	return true;
}