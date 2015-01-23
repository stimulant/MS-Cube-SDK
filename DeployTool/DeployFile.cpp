#include "stdafx.h"
#include "DeployFile.h"
#include <sstream>
#include <fstream>
#include <iostream>
#include <sys/stat.h>

#define DBOUT( s )            \
{                             \
   std::stringstream os_;    \
   os_ << s;                  \
   OutputDebugString( os_.str().c_str() );  \
}

DeployFile::DeployFile(std::string strFileName, std::string strPath, WIN32_FIND_DATA findData)
{
	mStrFileName = strFileName;
	mStrPath = strPath;
	mFindData = findData;

	DBOUT( "File: " << strPath << "\\" << strFileName << "\n" );
}

DeployFile::~DeployFile(void)
{
}

bool DeployFile::ServerSendToClient(std::string appName, std::string rootDirector, SOCKET hSocket)
{
	DBOUT("SERVER: start sending file: " << mStrFileName << "\n");

	// SEND COMMAND
	char command[32] = "SENDFILE";
	send(hSocket, command, 32, 0);
	char rec[32] = ""; 
	if (recv(hSocket, rec, 2, 0) <= 0)
		return false;

	// send file name
	send(hSocket, mStrFileName.c_str(), mStrFileName.length(), 0);
	if (recv(hSocket, rec, 2, 0) <= 0)
		return false;

	// send file path
	std::string adjustedFilePath = appName + "\\" + mStrPath;
	send(hSocket, adjustedFilePath.c_str(), adjustedFilePath.length(), 0);
	if (recv(hSocket, rec, 2, 0) <= 0)
		return false;

	// get file size
	struct stat stat_buf;
	std::string filePath = rootDirector + "\\" + mStrPath + "\\" + mStrFileName;
	if (stat(filePath.c_str(), &stat_buf) != 0)
		return false;
	int fileSize = stat_buf.st_size;

	// send file size
	char filesizeStr[10]; _ltoa((long)fileSize, filesizeStr, 10);
	send(hSocket, filesizeStr, 10, 0);
	if (recv(hSocket, rec, 2, 0) <= 0)
		return false;

	// send file
	FILE *fr = fopen(filePath.c_str(), "rb");
	int size = (unsigned int)fileSize;
	while(size > 0)
	{
		char buffer[1030];
		if(size>=1024)
		{
			fread(buffer, 1024, 1, fr);
			send(hSocket, buffer, 1024, 0);
			if (recv(hSocket, rec, 2, 0) <= 0)
				return false;
		}
		else
		{
			fread(buffer, size, 1, fr);
			buffer[size]='\0';
			send(hSocket, buffer, size, 0);
			if (recv(hSocket, rec, 2, 0) <= 0)
				return false;
		}
		size -= 1024;
	}
	fclose(fr);

	DBOUT("SERVER: finished sending file: " << mStrFileName << "\n");
	return true;
}

bool sendString(SOCKET hSocket, std::string string)
{
	char str[MAX_PATH] = "";
	strcpy(str, string.c_str());
	str[string.length()] = '\0';
	send(hSocket, str, MAX_PATH, 0);
}

bool DeployFile::ServerAskIfNeedsUpdate(std::string appName, std::string rootDirector, SOCKET hSocket, bool& doesNeedUpdate)
{
	// SEND COMMAND
	char command[32] = "ASKFILEUPDATE";
	send(hSocket, command, 32, 0);
	char rec[32] = ""; 
	if (recv(hSocket, rec, 2, 0) <= 0)
		return false;

	// send file name
	sendString(hSocket, mStrFileName);
	if (recv(hSocket, rec, 2, 0) <= 0)
		return false;

	// send file path
	std::string adjustedFilePath = appName + "\\" + mStrPath;
	sendString(hSocket, adjustedFilePath);
	if (recv(hSocket, rec, 2, 0) <= 0)
		return false;

	// send file modified date
	struct stat stat_buf;
	std::string filePath = rootDirector + "\\" + mStrPath + "\\" + mStrFileName;
	if (stat(filePath.c_str(), &stat_buf) != 0)
		return false;
	time_t modifiedTime = stat_buf.st_mtime;
	send(hSocket, (const char*)&modifiedTime, sizeof(time_t), 0);
	if (recv(hSocket, rec, 2, 0) <= 0)
		return false;
	
	doesNeedUpdate = (strcmp(rec, "YS") == 0);
	if (doesNeedUpdate)
		DBOUT("SERVER: filename: " << mStrFileName << " needs update" << "\n");

	return true;
}

bool DeployFile::ClientDoesFileNeedUpdate(SOCKET hSocket)
{
	char rec[50] = "";
	char filename[MAX_PATH] = "";
	char filepath[MAX_PATH] = "";

	// receive file name
	recv(hSocket, filename, MAX_PATH, 0);
	send(hSocket, "OK", 2, 0);

	// receive file path
	recv(hSocket, filepath, MAX_PATH, 0);
	send(hSocket, "OK", 2, 0);

	// receive file modified date
	time_t serverModifiedTime;
	recv(hSocket, (char*)&serverModifiedTime, sizeof(time_t), 0);
	
	//
	bool fileNeedsUpdating = false;
	std::string fullFilePathStr = filepath;
	if (fullFilePathStr != "")
		fullFilePathStr += "\\";
	fullFilePathStr += filename;
	struct stat stat_buf;
	if (stat(fullFilePathStr.c_str(), &stat_buf) != 0)
		fileNeedsUpdating = true;
	else
	{
		time_t clientModifiedTime = stat_buf.st_mtime;
		double diffTime = difftime(clientModifiedTime, serverModifiedTime);
		fileNeedsUpdating = (diffTime < 0.0);
	}

	if (fileNeedsUpdating)
		send(hSocket, "YS", 2, 0);
	else
		send(hSocket, "NO", 2, 0);

	return true;
}

bool DeployFile::ClientReceiveFile(SOCKET hSocket)
{
	char rec[50] = "";
	char filename[MAX_PATH] = "";
	char filepath[MAX_PATH] = "";

	// receive file name
	recv(hSocket, filename, MAX_PATH, 0);
	send(hSocket, "OK", 2, 0);

	// receive file path
	recv(hSocket, filepath, MAX_PATH, 0);
	send(hSocket, "OK", 2, 0);

	// receive file size
	int recs = recv(hSocket, rec, 10, 0);
	send(hSocket, "OK", 2, 0);
	rec[recs] = '\0';
	int size = atoi(rec);

	// create file directory if we don't have it already
	std::string filePathStr = filepath;
	unsigned int pos = 0;
	do
	{
		pos = filePathStr.find_first_of("\\/", pos + 1);
		CreateDirectory(filePathStr.substr(0, pos).c_str(), NULL);
	} while (pos != std::string::npos);

	// receive file
	std::string fullFilePathStr = filepath;
	if (fullFilePathStr != "")
		fullFilePathStr += "\\";
	fullFilePathStr += filename;
	FILE *fw = fopen(fullFilePathStr.c_str(), "wb");
	while(size > 0)
	{
		char buffer[1030];
		if(size>=1024)
		{
			recv(hSocket, buffer, 1024, 0);
			send(hSocket, "OK", 2, 0);
			fwrite(buffer, 1024, 1, fw);
		}
		else
		{
			recv(hSocket, buffer, size, 0);
			send(hSocket, "OK", 2, 0);
			buffer[size] = '\0';
			fwrite(buffer, size, 1, fw);
		}
		size -= 1024;
	}

	fclose(fw);
	DBOUT("CLIENT: received file: " << filename << "\n");
	return true;
}