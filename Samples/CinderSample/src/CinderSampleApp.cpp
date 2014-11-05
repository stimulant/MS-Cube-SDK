#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "KinectAPI.h"
#include "SocketHelper.h"

using namespace ci;
using namespace ci::app;
using namespace std;

#define MAXRECV 247815

class CinderSampleApp : public AppNative {
	bool mConnected;
	SOCKET mServerSocket;
	SOCKET mClientSocket;
	char* mRecvBuffer;

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
                 
        if(result == SOCKET_ERROR || result == 0)
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
		else
		{
			// parse received bytes
			app::console() << "Recv bytes:" << result << endl;
		}
	}
}

void CinderSampleApp::draw()
{
	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) ); 
}

CINDER_APP_NATIVE( CinderSampleApp, RendererGl )
