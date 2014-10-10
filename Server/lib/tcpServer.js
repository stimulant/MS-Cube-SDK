var net = require('net');
var binary = require('binary');

function start(host, port, kinectData) {

	// declare binary buffer
	var binaryBuffer = binary()
		.word8lu('command')
		.word16lu('dataLength')
		.word64lu('systemTime')
		.word8lu('skeleton1Present')
		.word8lu('skeleton2Present')
		.word8lu('skeleton3Present')
		.word8lu('skeleton4Present')
		.word8lu('skeleton5Present')
		.word8lu('skeleton6Present')
		.word16lu('skeletonCount')
		.tap(function (vars) {
			console.dir(vars);

			
		});

	net.createServer(function(socket) {

		console.log('CONNECTED: ' + socket.remoteAddress +':'+ socket.remotePort);
		socket.on('data', function(data) {
			binaryBuffer.write(data);
		});
		
		socket.on('close', function(data) {
			console.log('CLOSED: ' + socket.remoteAddress +' '+ socket.remotePort);
		});
		
	}).listen(port, host);
	console.log('TCP Server listening on ' + host +':'+ port);
}

exports.start = start;