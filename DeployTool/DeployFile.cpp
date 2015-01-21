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

	/*
	LARGE_INTEGER filesize;
	filesize.LowPart = mFindData.nFileSizeLow;
	filesize.HighPart = mFindData.nFileSizeHigh;
	mFileSize = filesize.QuadPart;
	*/

	DBOUT( "File: " << strPath << "\\" << strFileName << "\n" );
}

DeployFile::~DeployFile(void)
{
}

bool DeployFile::SendToClient(std::string rootDirector, SOCKET hSocket)
{
	// SEND COMMAND
	char command[32] = "SENDFILE";
	send(hSocket, command, 32, 0);
	char rec[32] = ""; 
	if (recv(hSocket, rec, 2, 0) <= 0)
		return false;

	// send file name
	send(hSocket, mStrFileName.c_str(), mStrFileName.length(), 0);
	DBOUT("SERVER: sent filename: " << mStrFileName << "\n");
	if (recv(hSocket, rec, 2, 0) <= 0)
		return false;

	// send file path
	send(hSocket, mStrPath.c_str(), mStrPath.length(), 0);
	if (recv(hSocket, rec, 2, 0) <= 0)
		return false;

	// get file size
	struct stat stat_buf;
	std::string filePath = rootDirector + "\\" + mStrPath + "\\" + mStrFileName;
	if (stat(filePath.c_str(), &stat_buf) != 0)
		return false;
	int fileSize = stat_buf.st_size;

	// send file size
	char filesizeStr[10]; _ltoa((long)mFileSize, filesizeStr, 10);
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
	return true;
}

bool DeployFile::AskIfNeedsUpdate(std::string rootDirector, SOCKET hSocket)
{
	// SEND COMMAND
	char command[32] = "DOESFILENEEDUPDATE";
	send(hSocket, command, 32, 0);
	char rec[32] = ""; 
	if (recv(hSocket, rec, 2, 0) <= 0)
		return false;

	// send file name
	send(hSocket, mStrFileName.c_str(), mStrFileName.length(), 0);
	DBOUT("SERVER: sent filename: " << mStrFileName << "\n");
	if (recv(hSocket, rec, 2, 0) <= 0)
		return false;

	// send file path
	send(hSocket, mStrPath.c_str(), mStrPath.length(), 0);
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

	return true;
}

bool DeployFile::DoesFileNeedUpdate(SOCKET hSocket)
{
	char rec[50] = "";
	char filename[_MAX_PATH] = "";
	char filepath[_MAX_PATH] = "";

	// receive file name
	recv(hSocket, filename, _MAX_PATH, 0);
	//DBOUT("CLIENT: received filename: " << filename << "\n");
	send(hSocket, "OK", 2, 0);
	//DBOUT("CLIENT: sent filename ack\n");

	// receive file path
	recv(hSocket, filepath, _MAX_PATH, 0);
	//DBOUT("CLIENT: received file path: " << filepath << "\n");
	send(hSocket, "OK", 2, 0);
	//DBOUT("CLIENT: sent file path ack\n");

	// receive file modified date
	time_t modifiedTime;
	recv(hSocket, (char*)&modifiedTime, sizeof(time_t), 0);
	//DBOUT("CLIENT: received file path: " << filepath << "\n");
	send(hSocket, "OK", 2, 0);
	//DBOUT("CLIENT: sent file path ack\n");

	//
	// !!! need to check file existence and mod data here
	//

	return true;
}

bool DeployFile::ReceiveFile(SOCKET hSocket)
{
	char rec[50] = "";
	char filename[_MAX_PATH] = "";
	char filepath[_MAX_PATH] = "";

	// receive file name
	recv(hSocket, filename, _MAX_PATH, 0);
	//DBOUT("CLIENT: received filename: " << filename << "\n");
	send(hSocket, "OK", 2, 0);
	//DBOUT("CLIENT: sent filename ack\n");

	// receive file path
	recv(hSocket, filepath, _MAX_PATH, 0);
	//DBOUT("CLIENT: received file path: " << filepath << "\n");
	send(hSocket, "OK", 2, 0);
	//DBOUT("CLIENT: sent file path ack\n");

	// receive file size
	int recs = recv(hSocket, rec, 10, 0);
	//DBOUT("CLIENT: received filesize: " << rec << "\n");
	send(hSocket, "OK", 2, 0);
	//DBOUT("CLIENT: sent filesize ack\n");
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
			//DBOUT("CLIENT: received file data\n");
			send(hSocket, "OK", 2, 0);
			//DBOUT("CLIENT: sent file data ack\n");
			fwrite(buffer, 1024, 1, fw);
		}
		else
		{
			recv(hSocket, buffer, size, 0);
			//DBOUT("CLIENT: received file data\n");
			send(hSocket, "OK", 2, 0);
			//DBOUT("CLIENT: sent file data ack\n");
			buffer[size] = '\0';
			fwrite(buffer, size, 1, fw);
		}
		size -= 1024;
	}

	fclose(fw);
	//DBOUT("CLIENT: closed file\n");
	return true;
}