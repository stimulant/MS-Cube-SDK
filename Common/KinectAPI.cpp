#include "KinectAPI.h"

#pragma pack(push, 1) // exact fit - no padding
struct BodiesUpdateHeader {
	char command;
	unsigned long dataLength;
	unsigned short bodyCount;
	UINT64 trackingIds[6];
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
					UINT64 trackingId = 0;
					if ( SUCCEEDED(pBody->get_TrackingId(&trackingId)) )
						header.trackingIds[i] = trackingId;
					header.bodyCount++;
				}
			}
		}
	}

	// write header
	header.command = 0;
	header.dataLength = 6 * JointType_Count * 3 * 4 + 8;
	memcpy(binary, &header, sizeof(BodiesUpdateHeader));
		
	// write body joints
	int byteOffset = sizeof(BodiesUpdateHeader);
	for (int i = 0; i < 6; ++i)
    {
		Joint joints[JointType_Count]; 
		if (ppBodies[i] != NULL && SUCCEEDED(ppBodies[i]->GetJoints(_countof(joints), joints)))
		{
			for (int j = 0; j < _countof(joints); ++j)
			{
				memcpy(&(binary[byteOffset]), &joints[j].Position.X, sizeof(float)); byteOffset += 4;
				memcpy(&(binary[byteOffset]), &joints[j].Position.Y, sizeof(float)); byteOffset += 4;
				memcpy(&(binary[byteOffset]), &joints[j].Position.Z, sizeof(float)); byteOffset += 4;
			}
		}
		else
		{
			float temp = 0.0f;
			for (int j = 0; j < JointType_Count; ++j)
			{
				memcpy(&(binary[byteOffset]), &temp, sizeof(float)); byteOffset += 4;
				memcpy(&(binary[byteOffset]), &temp, sizeof(float)); byteOffset += 4;
				memcpy(&(binary[byteOffset]), &temp, sizeof(float)); byteOffset += 4;
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

CommandType KinectAPI::BinaryToCommandAndLength(char* binary, int& binaryLength)
{
	// not a valid command 
	if (binary[0] != BodiesCommand && binary[0] != DepthCommand)
		return InvalidCommand;

	binaryLength = *(unsigned long*)(binary + 1);
	return (CommandType)binary[0];
}

bool KinectAPI::BinaryToDepth(char* binary, char* depthBuffer, int& width, int& height)
{
	// parse out width, height and depth from binary data
	DepthUpdateHeader depthHeader = *(DepthUpdateHeader*)(binary);
	width = depthHeader.width;
	height = depthHeader.height;
	memcpy(depthBuffer, binary + sizeof(DepthUpdateHeader), width * height);
	return true;
}

bool KinectAPI::BinaryToBodies(char* binary, UINT64* trackingIds, std::map< JointType, std::array<float, 3> > *jointPositions, int& bodyCount)
{
	// parse out number of bodies and joint positions from binary data
	BodiesUpdateHeader bodiesHeader = *(BodiesUpdateHeader*)(binary);
	bodyCount = bodiesHeader.bodyCount;

	for (int i=0; i < 6; i++)
		trackingIds[i] = bodiesHeader.trackingIds[i];

	char* binaryOffset = binary + sizeof(BodiesUpdateHeader);
	for (int i = 0; i < 6; ++i)
    {
		for (int j=0; j<JointType_Count; j++)
		{
			jointPositions[i][(JointType)j][0] = *(float*)(binaryOffset); binaryOffset += 4;
			jointPositions[i][(JointType)j][1] = *(float*)(binaryOffset); binaryOffset += 4;
			jointPositions[i][(JointType)j][2] = *(float*)(binaryOffset); binaryOffset += 4;
		}
	}

	return true;
}