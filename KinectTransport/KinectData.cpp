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
}

KinectData::~KinectData(void)
{
	if (mKinectSensor)
		mKinectSensor->Close();
}

bool KinectData::GetKinectBodies(IBody** ppBodies)
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
            hr = pBodyFrame->GetAndRefreshBodyData(BODY_COUNT, ppBodies);
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