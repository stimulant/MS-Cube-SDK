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
	io.sockets.emit('updateSkeleton', { skeletonCount: kinectData.skeletonCount, skeletons: kinectData.skeletons } );
}

function connect(socket)
{
	/*
	// control individual clients
	socket.on('startclient', this.startclient.bind(this) );
	socket.on('killclient', this.killclient.bind(this) );
	socket.on('restartclient', this.restartclient.bind(this) );

	// control all clients
	socket.on('startall', this.startall.bind(this));
	socket.on('killall', this.killall.bind(this));
	socket.on('restartall', this.restartall.bind(this));

	// user disconnects
	socket.on('disconnect', function(){});

	// params
	socket.on('updateparams', this.updateparams.bind(this));
	socket.on('setcurrentparam', this.setcurrentparam.bind(this));
	*/

	/*
	fs.readFile(__dirname + '/../web/Logo.png', function(err, buffer){
		console.log("sending data: " + buffer.length);
        socket.emit('updateDepth', { buffer: buffer });
    });*/
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