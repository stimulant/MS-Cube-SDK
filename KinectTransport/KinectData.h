#pragma once
#include "Kinect.h"
#include <string>

class KinectData
{
	// Kinect
	IKinectSensor*				mKinectSensor;
	bool						mKinectConnected;
	ICoordinateMapper*			mCoordinateMapper;
	IBodyFrameReader*			mBodyFrameReader;
	IDepthFrameReader*			mDepthFrameReader;
	WAITABLE_HANDLE				mBodyFrameEvent;
	WAITABLE_HANDLE				mDepthFrameEvent;

public:
	KinectData();
	~KinectData();

	bool GetKinectDepth(IDepthFrame** ppDepthFrame, int& nWidth, int& nHeight, UINT16 *&pBuffer, USHORT& nMinDepth, USHORT& nMaxDepth);
	bool GetKinectBodies(IBody** ppBodies);
};

