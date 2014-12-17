#include "stdafx.h"
#include "DeployFile.h"
#include <sstream>
#include <fstream>
#include <iostream>

#define DBOUT( s )            \
{                             \
   std::stringstream os_;    \
   os_ << s;                  \
   OutputDebugString( os_.str().c_str() );  \
}

DeployFile::DeployFile(std::string strFileName, std::string strPath, WIN32_FIND_DATA findData)
{
	m_strFileName = strFileName;
	m_strPath = strPath;
	m_FindData = findData;

	LARGE_INTEGER filesize;
	filesize.LowPart = m_FindData.nFileSizeLow;
	filesize.HighPart = m_FindData.nFileSizeHigh;
	m_FileSize = filesize.QuadPart;

	DBOUT( "File: " << strPath << "\\" << strFileName << "\n" );
}

DeployFile::~DeployFile(void)
{
}

bool DeployFile::SendToClient(SOCKET hSocket)
{
	// send file name
	send(hSocket, m_strFileName.c_str(), m_strFileName.length(), 0);
	char rec[32] = ""; 
	if (recv(hSocket, rec, 32, 0) <= 0)
		return false;

	// send file size
	char filesizeStr[10]; _ltoa((long)m_FileSize, filesizeStr, 10);
	send(hSocket, filesizeStr, strlen(filesizeStr), 0);
	if (recv(hSocket, rec, 32, 0) <= 0)
		return false;

	// send file
	std::string filePath = m_strPath + "\\" + m_strFileName;
	FILE *fr = fopen(filePath.c_str(), "rb");
	unsigned int size = (unsigned int)m_FileSize;
	while(size > 0)
	{
		char buffer[1030];
		if(size>=1024)
		{
			fread(buffer, 1024, 1, fr);
			send(hSocket, buffer, 1024, 0);
			if (recv(hSocket, rec, 32, 0) <= 0)
				return false;
		}
		else
		{
			fread(buffer, size, 1, fr);
			buffer[size]='\0';
			send(hSocket, buffer, size, 0);
			if (recv(hSocket, rec, 32, 0) <= 0)
				return false;
		}
		size -= 1024;
	}
	fclose(fr);
	return true;
}

bool DeployFile::ReceiveFile(SOCKET hSocket)
{
	char rec[50] = "";
	char filename[256] = "";

	// receive file name
	recv(hSocket, filename, 32, 0);
	send(hSocket, "OK", strlen("OK"), 0);

	// receive file size
	int recs = recv(hSocket, rec, 32, 0);
	send(hSocket, "OK", strlen("OK"), 0);
	rec[recs] = '\0';
	int size = atoi(rec);

	// receive file
	FILE *fw = fopen(filename, "wb");
	while(size > 0)
	{
		char buffer[1030];
		if(size>=1024)
		{
			recv(hSocket, buffer, 1024, 0);
			send(hSocket, "OK", strlen("OK"), 0);
			fwrite(buffer, 1024, 1, fw);
		}
		else
		{
			recv(hSocket, buffer, size, 0);
			send(hSocket, "OK", strlen("OK"), 0);
			buffer[size] = '\0';
			fwrite(buffer, size, 1, fw);
		}
		size -= 1024;
	}

	fclose(fw);
	return true;
}