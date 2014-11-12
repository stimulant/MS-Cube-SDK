var fs = require('fs');
var express = require('express');
var webapp = express();
var http = require('http');
var socketIO = require('socket.io');

var webserver;
var io;
var elapsed = 0.0;
var kinectData = {};

function update()
{
	elapsed += 1.0/30.0;

	if (kinectData.depthReady) {
		io.sockets.emit('updateDepth', { buffer: kinectData.depthBuffer });
		kinectData.depthReady = false;
	}
	if (kinectData.bodiesReady) {
		io.sockets.emit('updateBodies', { bodyCount: kinectData.bodyCount, bodyTrackingIds: kinectData.bodyTrackingIds, bodies: kinectData.bodies } );
		kinectData.bodiesReady = false;
	}
}

function connect(socket)
{
}

function start(host, port, kData)
{
	kinectData = kData;
	webserver = http.createServer(webapp);
	io = socketIO.listen(webserver);
	console.log( "Socket.io listening on port", port );
	webserver.listen( port );

	// routing
	webapp.get('/', function (req, res) {
		fs.readFile(__dirname + '/../web/servers.html', function read(err, data) {
			if (err) {
				console.log(err);
			}
			else {
				// replace server name and url in file
				var msg = data.toString();
				msg = msg.replace("%server_url%", "http://" + req.headers.host);
				msg = msg.replace("%server_name%", "Kinect Debug");
				res.send(msg);
			}
		});
	});
	webapp.use(express.static(__dirname + "/../web"));

	// user connects
	io.on('connection', connect);

	// start update loop
	setInterval(update, 1000.0/30.0);
}

exports.start          = start;