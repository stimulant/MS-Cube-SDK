#include "stdafx.h"
#include "KinectData.h"
#include "KinectAPI.h"

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

bool KinectData::SendBodiesUpdate(IBody** ppBodies)
{
	if (mSocket)
	{
		// turn bodies into a binary frame
		char binary[1208];
		int binarySize = KinectAPI::BodiesToBinary(ppBodies, binary);

		// send it out the socket
		int returnVal = send(mSocket, binary, binarySize, 0);
		if (returnVal == -1)
			return false;
	}
	return true;
}

bool KinectData::SendDepthUpdate(int nWidth, int nHeight, UINT16 *pBuffer, USHORT nMinDepth, USHORT nMaxDepth)
{
	if (mSocket)
	{
		// turn depth into a binary frame
		int binarySize = KinectAPI::DepthToBinary(nWidth, nHeight, pBuffer, nMinDepth, nMaxDepth, mDepthFrame);

		// send it out the socket
		int returnVal = send(mSocket, mDepthFrame, binarySize, 0);
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