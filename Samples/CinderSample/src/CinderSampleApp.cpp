#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "KinectAPI.h"
#include "SocketHelper.h"

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

	uint8_t depthBuffer[KINECT_DEPTH_WIDTH * KINECT_DEPTH_HEIGHT];
	Channel8u depthChannel;
	gl::Texture depthTexture;

  public:
	void setup();
	void mouseDown( MouseEvent event );	
	void update();
	void draw();
};

void CinderSampleApp::setup()
{
	mConnected = false;
	mRecvBuffer = new char[MAXRECV];
	depthChannel = Channel(KINECT_DEPTH_WIDTH, KINECT_DEPTH_HEIGHT, KINECT_DEPTH_WIDTH, 1, depthBuffer);
}

void CinderSampleApp::mouseDown( MouseEvent event )
{
}

void CinderSampleApp::update()
{
	// wait for connection
	if (!mConnected)
		mConnected = SocketHelper::WaitForClient(mServerSocket, mClientSocket, 3000);

	// receive data
	if (mConnected)
	{
		//Check if it was for closing , and also read the incoming message
        //recv does not place a null terminator at the end of the string (whilst printf %s assumes there is one).
        int result = recv(mClientSocket, mRecvBuffer, MAXRECV, 0);
                 
        if(result == SOCKET_ERROR)
        {
            int error_code = WSAGetLastError();
            if(error_code == WSAECONNRESET)
            {
                //Somebody disconnected , get his details and print
                app::console() << "Host disconnected unexpectedly" << endl;
                      
                //Close the socket and mark as 0 in list for reuse
                closesocket(mClientSocket);
				mConnected = false;
            }
            else
				app::console() << "Recv failed with error code:" << error_code << endl;
        }
		else if (result != 0)
		{
			// parse received bytes
			int binaryLength = 0;
			CommandType command = KinectAPI::BinaryToCommandAndLength(mRecvBuffer, binaryLength);
			app::console() << "Recv bytes:" << result << " command: " << command << " binaryLength: " << binaryLength <<endl;

			switch (command)
			{
				case BodiesCommand:

					break;
				case DepthCommand:
					{
						// copy data from binary and create texture from it
						int width, height;
						KinectAPI::BinaryToDepth((mRecvBuffer + 5), (char*)depthBuffer, width, height);
						depthTexture = gl::Texture(depthChannel);
					}
					break;
			}
		}
	}
}

void CinderSampleApp::draw()
{
	// clear out the window with black
	gl::clear(Color(0, 0, 0)); 
	gl::draw(depthTexture);
}

CINDER_APP_NATIVE( CinderSampleApp, RendererGl )
