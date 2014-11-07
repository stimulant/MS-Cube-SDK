var tcpServer = require('./lib/tcpServer.js');
var webServer = require('./lib/webServer.js');

// setup kinect data
var kinectData = {};
kinectData.skeletonCount = 0;
kinectData.skeletons = [];
kinectData.depthWidth = 0;
kinectData.depthHeight = 0;
kinectData.depthBuffer = new Buffer(217088);
kinectData.depthReady = false;

for (var s=0; s < 6; s++) {
	kinectData.skeletons[s] = [];
	for (var j=0; j < 25; j++)
	{
		kinectData.skeletons[s][j] = {};
		kinectData.skeletons[s][j].x = 0;
		kinectData.skeletons[s][j].y = 0;
		kinectData.skeletons[s][j].z = 0;
	}
}

tcpServer.start('0.0.0.0', 3000, kinectData);
webServer.start('0.0.0.0', 8000, kinectData);
