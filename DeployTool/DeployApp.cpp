#include "stdafx.h"
#include "DeployApp.h"
#include <windows.h>

DeployApp::DeployApp(std::string appDirectory, std::string appExecutable)
{
	mAppDirectory = appDirectory;
	mAppExecutable = appExecutable;

	AddDirectoryFiles(appDirectory, "");
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
			mfiles.push_back(new DeployFile(fileName, directory, ffd));
		}
	}
	while (FindNextFile(hFind, &ffd) != 0);
	FindClose(hFind);

	return true;
}

bool DeployApp::IsAppSelected(SOCKET clientSocket)
{
	// send command
	char command[32] = "ISAPPSELECTED";
	send(clientSocket, command, 32, 0);
	char rec[32] = ""; 
	if (recv(clientSocket, rec, 2, 0) <= 0)
		return false;

	// send executable name
	char executableName[128] = "";
	strcpy(executableName, mAppExecutable.c_str());
	executableName[mAppExecutable.length()] = '\0';
	send(clientSocket, executableName, 128, 0);
	if (recv(clientSocket, rec, 2, 0) <= 0)
		return false;
	
	return (strcmp(rec, "YS") == 0);
}

bool DeployApp::Update(SOCKET clientSocket)
{
	for (unsigned int i=0; i<mfiles.size(); i++)
	{
		// check if the app needs an update of this file
		if (!mfiles[i]->AskIfNeedsUpdate(mAppDirectory, clientSocket))
			continue;

		// send the file if so
		if (!mfiles[i]->SendToClient(mAppDirectory, clientSocket))
			return false;
		Sleep(100);
	}

	// start the app here if all done

	return true;
}

bool DeployApp::SendToClient(SOCKET clientSocket)
{
	for (unsigned int i=0; i<mfiles.size(); i++)
	{
		if (!mfiles[i]->SendToClient(mAppDirectory, clientSocket))
			return false;
	}
	return true;
}