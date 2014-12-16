#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "Kinect.h"
#include "KinectAPI.h"
#include "SocketHelper.h"
#include <process.h>

using namespace ci;
using namespace ci::app;
using namespace std;

#define MAXRECV 247815

#define KINECT_DEPTH_WIDTH 512
#define KINECT_DEPTH_HEIGHT 424

class CinderSampleApp : public AppNative {
	bool mConnected;
	SOCKET mServerSocket;
	SOCKET mClientSocket;
	char* mRecvBuffer;

	int mBodyCount;
	ULONG64 mTrackingIds[6];
	std::map< JointType, std::array<float, 3> > mJointPositions[6];

	uint8_t depthBuffer[KINECT_DEPTH_WIDTH * KINECT_DEPTH_HEIGHT];
	Channel8u depthChannel;
	gl::Texture depthTexture;

	// thread functions for handling sockets
	void socketThreadFunc();
	static void socketThreadFuncWrapper(void* o)
    {
        static_cast<CinderSampleApp*>(o)->socketThreadFunc();
    }
	void startSocketThread()
    {  
        _beginthread(&CinderSampleApp::socketThreadFuncWrapper, 0, static_cast<void*>(this));
    }

  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
};

void CinderSampleApp::socketThreadFunc()
{
	while (true)
	{
		// wait for connection
		if (!mConnected)
		{
			bool serverConnected = SocketHelper::CreateServerSocket(mServerSocket, 3000);
			if (serverConnected)
				mConnected = SocketHelper::WaitForClient(mServerSocket, mClientSocket);
		}

		// receive data
		if (mConnected)
		{
			//Check if it was for closing , and also read the incoming message
			//recv does not place a null terminator at the end of the string (whilst printf %s assumes there is one).
			int result = recv(mClientSocket, mRecvBuffer, MAXRECV, 0);
                 
			if(result == SOCKET_ERROR || result == 0)
			{
				int error_code = WSAGetLastError();
				if(error_code == WSAECONNRESET || result == 0)
				{
					//Somebody disconnected , get his details and print
					app::console() << "Host disconnected unexpectedly" << endl;
				}
				else
					app::console() << "Recv failed with error code:" << error_code << endl;

				//Close the socket and mark as 0 in list for reuse
				closesocket(mServerSocket);
				closesocket(mClientSocket);
				mConnected = false;
			}
			else
			{
				// parse received bytes
				int binaryLength = 0;
				CommandType command = KinectAPI::BinaryToCommandAndLength(mRecvBuffer, binaryLength);
				//app::console() << "Recv bytes:" << result << " command: " << command << " binaryLength: " << binaryLength <<endl;

				switch (command)
				{
					case BodiesCommand:
						{
							KinectAPI::BinaryToBodies(mRecvBuffer, mTrackingIds, mJointPositions, mBodyCount);
						}
						break;
					case DepthCommand:
						{
							// copy data from binary and create texture from it
							int width, height;
							KinectAPI::BinaryToDepth(mRecvBuffer, (char*)depthBuffer, width, height);
						}
						break;
				}
			}
		}
	}
}

void CinderSampleApp::setup()
{
	mConnected = false;
	mRecvBuffer = new char[MAXRECV];
	depthChannel = Channel(KINECT_DEPTH_WIDTH, KINECT_DEPTH_HEIGHT, KINECT_DEPTH_WIDTH, 1, depthBuffer);
	mBodyCount = 0;

	// start thread to listen for socket
	startSocketThread();
}

void CinderSampleApp::mouseDown( MouseEvent event )
{
}

void CinderSampleApp::update()
{
}

void CinderSampleApp::draw()
{
	// clear out the window with black
	gl::clear(Color(0, 0, 0));

	// draw depth
	depthTexture = gl::Texture(depthChannel);
	gl::draw(depthTexture, Rectf(0.0f, 0.0f, (float)app::getWindowWidth(), (float)app::getWindowHeight()));

	// draw bodies
	for (int i=0; i<6; i++)
	{
		if (mTrackingIds[i] != 0)
		{
			gl::color(ColorA::white());
			for (int j=0; j<JointType_Count; j++)
				gl::drawStrokedCircle(Vec2f(mJointPositions[i][(JointType)j][0], 1.0f - mJointPositions[i][(JointType)j][1]) * 
					Vec2f((float)app::getWindowWidth()/2.0f, (float)app::getWindowHeight()/2.0f) + 
					Vec2f((float)app::getWindowWidth()/2.0f, 0.0f), 5.0f);
		}
	}
}

CINDER_APP_NATIVE( CinderSampleApp, RendererGl )
