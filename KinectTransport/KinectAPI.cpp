#include "stdafx.h"
#include "KinectAPI.h"

#pragma pack(push, 1) // exact fit - no padding
struct BodiesUpdateHeader {
	char command;
	unsigned long dataLength;
	char bodiesPresent[6];
	unsigned short bodyCount;
};
#pragma pack(pop)

int KinectAPI::BodiesToBinary(IBody** ppBodies, char* binary)
{
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
	memcpy(binary, &header, sizeof(BodiesUpdateHeader));
		
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
					memcpy(&(binary[byteOffset]), &joints[j].Position.X, sizeof(float)); byteOffset += 4;
					memcpy(&(binary[byteOffset]), &joints[j].Position.Y, sizeof(float)); byteOffset += 4;
					memcpy(&(binary[byteOffset]), &joints[j].Position.Z, sizeof(float)); byteOffset += 4;
				}
			}
		}
	}
	return byteOffset;
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

int KinectAPI::DepthToBinary(int nWidth, int nHeight, UINT16 *pBuffer, USHORT nMinDepth, USHORT nMaxDepth, char* binary)
{
	DepthUpdateHeader header;
	memset(&header, 0, sizeof(DepthUpdateHeader));

	// write header
	header.command = 1;
	header.dataLength = nWidth * nHeight + 4;
	header.width = nWidth;
	header.height = nHeight;
	memcpy(binary, &header, sizeof(DepthUpdateHeader));
	int byteOffset = sizeof(DepthUpdateHeader);

    // end pixel is start + width*height - 1
    const UINT16* pBufferEnd = pBuffer + (nWidth * nHeight);

    while (pBuffer < pBufferEnd)
    {
        USHORT depth = *pBuffer;
		if (depth < nMinDepth || depth > nMaxDepth)
			binary[byteOffset] = 0;
		else
		{
			//pDepthFrame[byteOffset] = static_cast<char>((depth >= nMinDepth) && (depth <= nMaxDepth) ? (depth % 256) : 0);
			binary[byteOffset] = static_cast<char>(((float)depth/(float)nMaxDepth) * 255);
		}
		byteOffset++;
		pBuffer++;
	}

	// RLEncode our frame
	//int rlLength = RLEncode(pDepthFrame, byteOffset, pDepthEncodedFrame);
	//DebugOutput("%d, %d\n", byteOffset, rlLength);
	return byteOffset;
}