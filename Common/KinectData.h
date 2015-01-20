#pragma once
#include "Kinect.h"
#include <string>
#include <map>
#include <array>

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
	bool GetKinectBodies(UINT64* trackingIds, std::map< JointType, std::array<float, 3> > *jointPositions, std::pair<HandState, HandState> *handStates, int& bodyCount);
};

