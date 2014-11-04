#include "stdafx.h"
#include "KinectData.h"

#define KINECT_DEPTH_WIDTH 512
#define KINECT_DEPTH_HEIGHT 424

KinectData::KinectData()
{
	// setup Kinect
	HRESULT hr = GetDefaultKinectSensor(&mKinectSensor);
    if (SUCCEEDED(hr) && mKinectSensor)
    {
        hr = mKinectSensor->Open();
        if (SUCCEEDED(hr))
            hr = mKinectSensor->get_CoordinateMapper(&mCoordinateMapper);

		// setup body frame
		IBodyFrameSource* pBodyFrameSource = NULL;
        if (SUCCEEDED(hr))
            hr = mKinectSensor->get_BodyFrameSource(&pBodyFrameSource);
        if (SUCCEEDED(hr))
            hr = pBodyFrameSource->OpenReader(&mBodyFrameReader);
		if (SUCCEEDED(hr))
			hr = mBodyFrameReader->SubscribeFrameArrived(&mBodyFrameEvent);
		if (pBodyFrameSource != NULL)
			pBodyFrameSource->Release();

		// setup depth frame
		IDepthFrameSource* pDepthFrameSource = NULL;
		if (SUCCEEDED(hr))
            hr = mKinectSensor->get_DepthFrameSource(&pDepthFrameSource);
        if (SUCCEEDED(hr))
            hr = pDepthFrameSource->OpenReader(&mDepthFrameReader);
		if (SUCCEEDED(hr))
			hr = mDepthFrameReader->SubscribeFrameArrived(&mDepthFrameEvent);
		if (pDepthFrameSource != NULL)
			pDepthFrameSource->Release();
    }

	// create heap storage for depth pixel data in RGBX format
    mDepthRGBX = new RGBQUAD[KINECT_DEPTH_WIDTH * KINECT_DEPTH_HEIGHT];

	// maximum size of frame is 247815 bytes (512x424 + 7)
	mDepthFrame = new char[247815];
	mDepthEncodedFrame = new char[247815*2];
}

KinectData::~KinectData(void)
{
	if (mKinectSensor)
		mKinectSensor->Close();
}

bool KinectData::UpdateKinect(bool sendBodiesData, bool sendDepthData)
{
	if (sendBodiesData)
	{
		if (!UpdateKinectBodies())
			return false;
	}
	if (sendDepthData)
	{
		if (!UpdateKinectDepth())
			return false;
	}
	return true;
}

bool KinectData::UpdateKinectBodies()
{
	bool connected = true;

	DWORD dwResult = WaitForSingleObjectEx(reinterpret_cast<HANDLE>(mBodyFrameEvent), 0, FALSE);
    if (WAIT_OBJECT_0 != dwResult)
		return connected;
	if (!mBodyFrameEvent)
        return connected;

    IBodyFrame* pBodyFrame = NULL;
    HRESULT hr = mBodyFrameReader->AcquireLatestFrame(&pBodyFrame);
    if (SUCCEEDED(hr))
    {
        INT64 nTime = 0;
        hr = pBodyFrame->get_RelativeTime(&nTime);
        IBody* ppBodies[BODY_COUNT] = {0};

        if (SUCCEEDED(hr))
            hr = pBodyFrame->GetAndRefreshBodyData(_countof(ppBodies), ppBodies);
		
        if (SUCCEEDED(hr))
		{
			// send bodes as an update
			connected = SendBodiesUpdate(ppBodies);
		}
		
        for (int i = 0; i < _countof(ppBodies); ++i)
		{
			if (ppBodies[i] != NULL)
				ppBodies[i]->Release();
		}
    }
	
	if (pBodyFrame != NULL)
		pBodyFrame->Release();

	return connected;
}

bool KinectData::UpdateKinectDepth()
{
	bool connected = true;

	DWORD dwResult = WaitForSingleObjectEx(reinterpret_cast<HANDLE>(mDepthFrameEvent), 0, FALSE);
    if (WAIT_OBJECT_0 != dwResult)
		return connected;
	if (!mDepthFrameEvent)
        return connected;

    IDepthFrame* pDepthFrame = NULL;
    HRESULT hr = mDepthFrameReader->AcquireLatestFrame(&pDepthFrame);

	INT64 nTime = 0;
    IFrameDescription* pFrameDescription = NULL;
    int nWidth = 0;
    int nHeight = 0;
    USHORT nDepthMinReliableDistance = 0;
    USHORT nDepthMaxReliableDistance = 0;
    UINT nBufferSize = 0;
    UINT16 *pBuffer = NULL;

    if (SUCCEEDED(hr))
        hr = pDepthFrame->get_RelativeTime(&nTime);

	if (SUCCEEDED(hr))
		hr = pDepthFrame->get_FrameDescription(&pFrameDescription);

	if (SUCCEEDED(hr))
		hr = pFrameDescription->get_Width(&nWidth);

	if (SUCCEEDED(hr))
		hr = pFrameDescription->get_Height(&nHeight);

	if (SUCCEEDED(hr))
		hr = pDepthFrame->get_DepthMinReliableDistance(&nDepthMinReliableDistance);

	if (SUCCEEDED(hr))
		hr = pDepthFrame->get_DepthMaxReliableDistance(&nDepthMaxReliableDistance);

	if (SUCCEEDED(hr))
		hr = pDepthFrame->AccessUnderlyingBuffer(&nBufferSize, &pBuffer);

	if (SUCCEEDED(hr))
	{
		// send depth as an update
		connected = SendDepthUpdate(nWidth, nHeight, pBuffer, nDepthMinReliableDistance, nDepthMaxReliableDistance);
	}

	if (pDepthFrame != NULL)
		pDepthFrame->Release();

	return connected;
}

#pragma pack(push, 1) // exact fit - no padding
struct BodiesUpdateHeader {
	char command;
	unsigned long dataLength;
	char bodiesPresent[6];
	unsigned short bodyCount;
};
#pragma pack(pop)

bool KinectData::SendBodiesUpdate(IBody** ppBodies)
{
	if (mSocket)
	{
		// maximum size of frame is 1208 bytes
		char frame[1208];

		BodiesUpdateHeader header;
		memset(&header, 0, sizeof(BodiesUpdateHeader));

		// first get bodies presence
		for (int i = 0; i < BODY_COUNT; ++i)
        {
            IBody* pBody = ppBodies[i];
			if (pBody)
            {
				BOOLEAN bTracked = false;
				if ( SUCCEEDED(pBody->get_IsTracked(&bTracked)) )
				{
					if (bTracked)
					{
						header.bodiesPresent[i] = 1;
						header.bodyCount++;
					}
				}
			}
		}

		// write header
		header.command = 0;
		header.dataLength = header.bodyCount * JointType_Count * 3 * 4 + 8;
		memcpy(frame, &header, sizeof(BodiesUpdateHeader));
		
		// write body joints
		int byteOffset = sizeof(BodiesUpdateHeader);
		for (int i = 0; i < 6; ++i)
        {
			// only send body data if this body is present
			if (header.bodiesPresent[i] == 1)
			{
				Joint joints[JointType_Count]; 
				HRESULT hr = ppBodies[i]->GetJoints(_countof(joints), joints);
				if (SUCCEEDED(hr))
				{
					for (int j = 0; j < _countof(joints); ++j)
					{
						memcpy(&(frame[byteOffset]), &joints[j].Position.X, sizeof(float)); byteOffset += 4;
						memcpy(&(frame[byteOffset]), &joints[j].Position.Y, sizeof(float)); byteOffset += 4;
						memcpy(&(frame[byteOffset]), &joints[j].Position.Z, sizeof(float)); byteOffset += 4;
					}
				}
			}
		}

		// send it out the socket
		int returnVal = send(mSocket, frame, byteOffset, 0);
		if (returnVal == -1)
			return false;
	}
	return true;
}

int RLEncode(char *data, int dataLength, char* output)
{
	int index,count;
	char current;

	current=data[0];
	count =0;
	index =0;

	for(int i=0; i<=dataLength; i++)
	{
		if(data[i] != current || i >= 255)
		{
			output[index++]=current;
			output[index++]=count + '0';
			current = data[i];
			count = 0;
		}
		count++;
	}
	return index;
}

#pragma pack(push, 1) // exact fit - no padding
struct DepthUpdateHeader {
	char command;
	unsigned long dataLength;
	unsigned short width;
	unsigned short height;
};
#pragma pack(pop)

bool KinectData::SendDepthUpdate(int nWidth, int nHeight, UINT16 *pBuffer, USHORT nMinDepth, USHORT nMaxDepth)
{
	if (mSocket)
	{
		DepthUpdateHeader header;
		memset(&header, 0, sizeof(DepthUpdateHeader));

		// write header
		header.command = 1;
		header.dataLength = nWidth * nHeight + 4;
		header.width = nWidth;
		header.height = nHeight;
		memcpy(mDepthFrame, &header, sizeof(DepthUpdateHeader));
		int byteOffset = sizeof(DepthUpdateHeader);

        // end pixel is start + width*height - 1
        const UINT16* pBufferEnd = pBuffer + (nWidth * nHeight);

        while (pBuffer < pBufferEnd)
        {
            USHORT depth = *pBuffer;
			if (depth < nMinDepth || depth > nMaxDepth)
				mDepthFrame[byteOffset] = 0;
			else
			{
				//pDepthFrame[byteOffset] = static_cast<char>((depth >= nMinDepth) && (depth <= nMaxDepth) ? (depth % 256) : 0);
				mDepthFrame[byteOffset] = static_cast<char>(((float)depth/(float)nMaxDepth) * 255);
			}
			byteOffset++;
			pBuffer++;
		}

		// RLEncode our frame
		//int rlLength = RLEncode(pDepthFrame, byteOffset, pDepthEncodedFrame);
		//DebugOutput("%d, %d\n", byteOffset, rlLength);

		// send it out the socket
		int returnVal = send(mSocket, mDepthFrame, byteOffset, 0);
		if (returnVal == -1)
			return false;
	}
	return true;
}

bool KinectData::ConnectToHost(int PortNo, const char* IPAddress)
{
    // Start winsock
    WSADATA wsadata;
    int error = WSAStartup(0x0202, &wsadata);
    if (error)
        return false;

    // Make sure we have winsock v2
    if (wsadata.wVersion != 0x0202)
    {
        WSACleanup();
        return false;
    }

    // Setup socket address
    SOCKADDR_IN target;
    target.sin_family = AF_INET;
    target.sin_port = htons (PortNo);
    target.sin_addr.s_addr = inet_addr(IPAddress);

	// Create socket
    mSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (mSocket == INVALID_SOCKET)
        return false;

    // Connect
    if (connect(mSocket, (SOCKADDR *)&target, sizeof(target)) == SOCKET_ERROR)
        return false;
    else
        return true;
}

void KinectData::CloseConnection()
{
    if (mSocket)
        closesocket(mSocket);
    WSACleanup();
}