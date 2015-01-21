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

bool DeployApp::SendToClient(SOCKET clientSocket)
{
	for (unsigned int i=0; i<mfiles.size(); i++)
	{
		if (!mfiles[i]->SendToClient(mAppDirectory, clientSocket))
			return false;
	}
	return true;
}