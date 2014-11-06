#pragma once
#include "Kinect.h"

enum CommandType
{
	BodiesCommand,
	DepthCommand,
	InvalidCommand
};

class KinectAPI
{
public:
	static int BodiesToBinary(IBody** ppBodies, char* binary);
	static int DepthToBinary(int nWidth, int nHeight, UINT16 *pBuffer, USHORT nMinDepth, USHORT nMaxDepth, char* binary);
	static CommandType BinaryToCommandAndLength(char* binary, int& binaryLength);
	static bool BinaryToDepth(char* binary, char* depthBuffer, int& width, int& height);
};

