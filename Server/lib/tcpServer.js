var net = require('net');

function start(host, port, kinectData) {

	net.createServer(function(socket) {
		console.log('CONNECTED: ' + socket.remoteAddress +':'+ socket.remotePort);
		var remoteAddress = socket.remoteAddress;
		var remotePort = socket.remotePort;

		socket.on('data', function(data) {
			//console.log(data.length);
			var offset = 0;

			// parse out main header
			var command		= data.readUInt8(offset); offset += 1;
			var dataLength	= data.readUInt16LE(offset); offset += 2;
			offset += 6;	// skeletons preset
			kinectData.skeletonCount = data.readUInt16LE(offset); offset += 2;

			// parse joint data
			for (var s=0; s<kinectData.skeletonCount; s++) {
				for (var j=0; j<25; j++) {
					kinectData.skeletons[s][j].x = data.readFloatLE(offset);	offset += 4;
					kinectData.skeletons[s][j].y = data.readFloatLE(offset);	offset += 4;
					kinectData.skeletons[s][j].z = data.readFloatLE(offset);	offset += 4;
				}
			}
		});
		
		socket.on('close', function(data) {
			console.log('CLOSED: ' + remoteAddress +' '+ remotePort);
		});
		
	}).listen(port, host);
	console.log('TCP Server listening on ' + host +':'+ port);
}

exports.start = start;