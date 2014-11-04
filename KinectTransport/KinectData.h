#pragma once
#include "Kinect.h"
#include <winsock.h>
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

	// Depth
	RGBQUAD*					mDepthRGBX;
	char*						mDepthFrame;
	char*						mDepthEncodedFrame;
	
	// Socket
	SOCKET						mSocket;

	bool UpdateKinectBodies();
	bool UpdateKinectDepth();

	bool SendDepthUpdate(int nWidth, int nHeight, UINT16 *pBuffer, USHORT nMinDepth, USHORT nMaxDepth);
	bool SendBodiesUpdate(IBody** ppBodies);


public:
	KinectData();
	~KinectData();

	bool UpdateKinect(bool sendBodiesData, bool sendDepthData);
	bool ConnectToHost(int PortNo, const char* IPAddress);
	void CloseConnection();
};

