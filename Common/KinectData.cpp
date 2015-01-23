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
}

KinectData::~KinectData(void)
{
	if (mKinectSensor)
		mKinectSensor->Close();
}

bool KinectData::GetKinectBodies(UINT64* trackingIds, std::map< JointType, std::array<float, 3> > *jointPositions, std::map< JointType, std::array<float, 4> > *jointOrientations, std::pair<HandState, HandState> *handStates, int& bodyCount)
{
	DWORD dwResult = WaitForSingleObjectEx(reinterpret_cast<HANDLE>(mBodyFrameEvent), 0, FALSE);
    if (WAIT_OBJECT_0 != dwResult)
		return false;
	if (!mBodyFrameEvent)
        return false;

    IBodyFrame* pBodyFrame = NULL;
    HRESULT hr = mBodyFrameReader->AcquireLatestFrame(&pBodyFrame);
    if (SUCCEEDED(hr))
    {
        INT64 nTime = 0;
        hr = pBodyFrame->get_RelativeTime(&nTime);

        if (SUCCEEDED(hr))
		{
			IBody* ppBodies[BODY_COUNT] = {0};
            hr = pBodyFrame->GetAndRefreshBodyData(BODY_COUNT, ppBodies);
			
			for (int i = 0; i < BODY_COUNT; ++i)
			{
				trackingIds[i] = 0;
				IBody* pBody = ppBodies[i];
				if (pBody)
				{
					BOOLEAN bTracked = false;
					if ( SUCCEEDED(pBody->get_IsTracked(&bTracked)) )
					{
						if (bTracked)
						{
							UINT64 trackingId = 0;
							if ( SUCCEEDED(pBody->get_TrackingId(&trackingId)) )
							{
								// get tracking ids
								trackingIds[i] = trackingId;
								bodyCount++;

								// get body joints
								Joint joints[JointType_Count]; 
								if (SUCCEEDED(pBody->GetJoints(_countof(joints), joints)))
								{
									for (int j = 0; j < _countof(joints); ++j)
									{
										jointPositions[i][(JointType)j][0] = joints[j].Position.X;
										jointPositions[i][(JointType)j][1] = joints[j].Position.Y;
										jointPositions[i][(JointType)j][2] = joints[j].Position.Z;
									}
								}

								// get hand states
								pBody->get_HandLeftState(&(handStates[i].first));
								pBody->get_HandRightState(&(handStates[i].second));

								// get joint orientations
								JointOrientation jointO[JointType_Count];
								if (SUCCEEDED(pBody->GetJointOrientations(_countof(jointO), jointO)))
								{
									for (int j = 0; j < _countof(jointO); ++j)
									{
										jointOrientations[i][(JointType)j][0] = jointO[j].Orientation.x;
										jointOrientations[i][(JointType)j][1] = jointO[j].Orientation.y;
										jointOrientations[i][(JointType)j][2] = jointO[j].Orientation.z;
										jointOrientations[i][(JointType)j][3] = jointO[j].Orientation.w;
									}
								}
								
							}
						}
					}
				}
			}

			// release bodies data
			for (int i = 0; i < _countof(ppBodies); ++i)
			{
				if (ppBodies[i] != NULL)
					ppBodies[i]->Release();
			}
		}
    }
	
	if (pBodyFrame != NULL)
		pBodyFrame->Release();

	return SUCCEEDED(hr);
}

bool KinectData::GetKinectDepth(IDepthFrame **ppDepthFrame, int& nWidth, int& nHeight, UINT16 *&pBuffer, USHORT& nMinDepth, USHORT& nMaxDepth)
{
	DWORD dwResult = WaitForSingleObjectEx(reinterpret_cast<HANDLE>(mDepthFrameEvent), 0, FALSE);
    if (WAIT_OBJECT_0 != dwResult)
		return false;
	if (!mDepthFrameEvent)
        return false;
    HRESULT hr = mDepthFrameReader->AcquireLatestFrame(ppDepthFrame);

	INT64 nTime = 0;
    IFrameDescription* pFrameDescription = NULL;
    UINT nBufferSize = 0;

    if (SUCCEEDED(hr))
        hr = (*ppDepthFrame)->get_RelativeTime(&nTime);

	if (SUCCEEDED(hr))
		hr = (*ppDepthFrame)->get_FrameDescription(&pFrameDescription);

	if (SUCCEEDED(hr))
		hr = pFrameDescription->get_Width(&nWidth);

	if (SUCCEEDED(hr))
		hr = pFrameDescription->get_Height(&nHeight);

	if (SUCCEEDED(hr))
		hr = (*ppDepthFrame)->get_DepthMinReliableDistance(&nMinDepth);

	if (SUCCEEDED(hr))
		hr = (*ppDepthFrame)->get_DepthMaxReliableDistance(&nMaxDepth);

	if (SUCCEEDED(hr))
		hr = (*ppDepthFrame)->AccessUnderlyingBuffer(&nBufferSize, &pBuffer);

	return SUCCEEDED(hr);
}