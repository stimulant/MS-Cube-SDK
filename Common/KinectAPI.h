#pragma once
#include "Kinect.h"

class KinectAPI
{
public:
	static int BodiesToBinary(IBody** ppBodies, char* binary);
	static int DepthToBinary(int nWidth, int nHeight, UINT16 *pBuffer, USHORT nMinDepth, USHORT nMaxDepth, char* binary);
};

