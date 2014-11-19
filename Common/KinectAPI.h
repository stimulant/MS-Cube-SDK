#pragma once
#include "Kinect.h"
#include <map>
#include <array>

enum CommandType
{
	BodiesCommand,
	DepthCommand,
	InvalidCommand
};

class KinectAPI
{
public:
	static int BodiesToBinary(UINT64* trackingIds, std::map< JointType, std::array<float, 3> > *jointPositions, int bodyCount, char* binary);
	static int DepthToBinary(int nWidth, int nHeight, UINT16 *pBuffer, USHORT nMinDepth, USHORT nMaxDepth, char* binary);
	static CommandType BinaryToCommandAndLength(char* binary, int& binaryLength);
	static bool BinaryToDepth(char* binary, char* depthBuffer, int& width, int& height);
	static bool BinaryToBodies(char* binary, UINT64* trackingIds, std::map< JointType, std::array<float, 3> > *jointPositions, int& bodyCount);
};

