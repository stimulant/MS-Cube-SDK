#include "stdafx.h"
#include "DeployApp.h"
#include <windows.h>

DeployApp::DeployApp(std::string appDirectory, std::string appExecutable)
{
	m_appDirectory = appDirectory;
	m_appExecutable = appExecutable;

	AddDirectoryFiles(appDirectory);
}

DeployApp::~DeployApp(void)
{
}

bool DeployApp::AddDirectoryFiles(std::string directory)
{
	// go through directory and add files
	WIN32_FIND_DATA ffd;
	LARGE_INTEGER filesize;
	//TCHAR szDir[MAX_PATH];

   // Prepare string for use with FindFile functions.  First, copy the
   // string to a buffer, then append '\*' to the directory name.
   //StringCchCopy(szDir, MAX_PATH, argv[1]);
   //StringCchCat(szDir, MAX_PATH, TEXT("\\*"));

   // List all the files in the directory with some info about them.
	HANDLE hFind = FindFirstFile(directory.c_str(), &ffd);
	if (INVALID_HANDLE_VALUE == hFind) 
		return false;
	do
	{
		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			_tprintf(TEXT("  %s   <DIR>\n"), ffd.cFileName);

			// iterate over directory files
			std::string directoryName = directory + "\\" + ffd.cFileName + "\\";
			//AddDirectoryFiles(directoryName);
		}
		else
		{
			filesize.LowPart = ffd.nFileSizeLow;
			filesize.HighPart = ffd.nFileSizeHigh;
			_tprintf(TEXT("  %s   %ld bytes\n"), ffd.cFileName, filesize.QuadPart);
		}
	}
	while (FindNextFile(hFind, &ffd) != 0);
	FindClose(hFind);

	return true;
}
