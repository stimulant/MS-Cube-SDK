var net = require('net');

function parseSkeleton(kinectData, data) {
	var offset = 0;
	offset += 6;	// skeletons preset
	kinectData.skeletonCount = data.readUInt16LE(offset); offset += 2;
	//console.log("parsing skeletons: " + kinectData.skeletonCount);

	// parse joint data
	for (var s=0; s<kinectData.skeletonCount; s++) {
		for (var j=0; j<25; j++) {
			kinectData.skeletons[s][j].x = data.readFloatLE(offset);	offset += 4;
			kinectData.skeletons[s][j].y = data.readFloatLE(offset);	offset += 4;
			kinectData.skeletons[s][j].z = data.readFloatLE(offset);	offset += 4;
		}
	}
}

function parseDepth(kinectData, data) {
	var offset = 2;
	//console.log("parsing depth");
	kinectData.depthWidth = data.readUInt16LE(offset); offset += 2;
	kinectData.depthHeight = data.readUInt16LE(offset); offset += 2;

	// copy buffer data to kinectData
	//console.log("parsing depth: " + kinectData.depthWidth + ", " + kinectData.depthHeight);
	data.copy(kinectData.depthBuffer, 0, offset, kinectData.depthWidth * kinectData.depthHeight + offset);
	offset += kinectData.depthWidth * kinectData.depthHeight;
	kinectData.depthReady = true;
}

function start(host, port, kinectData) {

	net.createServer(function(socket) {
		console.log('CONNECTED: ' + socket.remoteAddress +':'+ socket.remotePort);
		var remoteAddress = socket.remoteAddress;
		var remotePort = socket.remotePort;

		// maximum size of frame is 247815 bytes (512x424 + 7)
		var currentCommand = -1;
		var parseBuffer = new Buffer(247815);
		var parseDataOffset = 0;
		var parseDataLength = 0;

		socket.on('data', function(data) {
			//console.log("reading: " + data.length + " for command " + currentCommand);
			var dataOffset = 0;

			// if we don't have a command yet
			if (currentCommand == -1)
			{
				// parse out main header
				currentCommand	= data.readUInt8(dataOffset); dataOffset += 1;
				parseDataLength	= data.readUInt32LE(dataOffset); dataOffset += 2;
			}

			// keep adding to parseBuffer until we have entire frame
			var copyLength = Math.min(parseDataLength - parseDataOffset, data.length - dataOffset);
			//console.log("copying " + copyLength + " data from buffer with length " + data.length);
			data.copy(parseBuffer, parseDataOffset, dataOffset, copyLength);
			parseDataOffset += copyLength;

			// if we are done receiving the buffer, go ahead and parse
			if (parseDataOffset >= parseDataLength) {
				// assume we have a command now
				if (currentCommand == 0)
					parseSkeleton(kinectData, parseBuffer);
				else if (currentCommand == 1)
					parseDepth(kinectData, parseBuffer);

				// reset command
				currentCommand = -1; parseDataOffset = 0; parseDataLength = 0;
			}	
		});
		
		socket.on('close', function(data) {
			console.log('CLOSED: ' + remoteAddress +' '+ remotePort);
		});
		
	}).listen(port, host);
	console.log('TCP Server listening on ' + host +':'+ port);
}

exports.start = start;