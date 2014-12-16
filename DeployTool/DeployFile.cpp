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
	char rec[32] = ""; recv(hSocket, rec, 32, 0);

	// send file size
	char filesizeStr[10]; _ltoa((long)m_FileSize, filesizeStr, 10);
	send(hSocket, filesizeStr, strlen(filesizeStr), 0);
	recv(hSocket, rec, 32, 0);

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
			recv(hSocket, rec, 32, 0);
		}
		else
		{
			fread(buffer, size, 1, fr);
			buffer[size]='\0';
			send(hSocket, buffer, size, 0);
			recv(hSocket, rec, 32, 0);
		}
		size -= 1024;
	}
	fclose(fr);
	return true;

	/*
	DeployFileHeader header;
	LARGE_INTEGER filesize;
	memset(&header, 0, sizeof(DeployFileHeader));

	// setup file header
	strcpy(header.fileName, m_strFileName.c_str());
	strcpy(header.path, m_strPath.c_str());
	filesize.LowPart = m_FindData.nFileSizeLow;
	filesize.HighPart = m_FindData.nFileSizeHigh;
	header.fileLength = filesize.QuadPart;

	// send file header
	if (send(hSocket, (const char *)&header, sizeof(DeployFileHeader), 0) == -1)
	{
		return false; // disconnected
	}

	// send file data
	std::string filePath = m_strPath + "\\" + m_strFileName;
	std::ifstream inFile(filePath.c_str(), std::ifstream::binary);
	inFile.seekg(0, std::ifstream::beg);
	int pos = inFile.tellg();
	while (pos != -1)
	{
		char *p = new char[1024];
		memset(p, 0, 1024);
		inFile.read(p, 1024);

		printf("%ld\n", inFile.gcount());
		int n = send(hSocket, p, 1024, 0);
		if (n < 0)
		{
			DBOUT( "ERROR writing to socket\n" );
			return false;
		}
		else
			DBOUT( "File sent: " << m_strPath << "\\" << m_strFileName << "\n" );

		delete p;
	}
	inFile.close();
	return true;
	*/
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