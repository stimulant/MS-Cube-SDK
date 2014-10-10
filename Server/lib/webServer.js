var fs = require('fs');
var express = require('express');
var webapp = express();
var http = require('http');
var socketIO = require('socket.io');

var webserver;
var io;

function update()
{
	/*
	this.elapsed += 1.0/30.0;
	
	if (this.updateSocket++ > 10)
	{
		var data = {};
		data.clients = this.clients;
		data.config = this.config;
		this.io.sockets.emit('updateclients', data);
		updateSocket = 0;
	}

	// update heartbeat for clients
	for (var i in this.clients )
		this.updateClientHeartbeat(this.clients[i]);
	*/
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
}

function start(host, port)
{
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