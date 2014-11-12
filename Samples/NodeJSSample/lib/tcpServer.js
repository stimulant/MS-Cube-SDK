var net = require('net');

function readInt64LEasFloat(buffer, offset) {
  var low = buffer.readUInt32LE(offset + 4);
  var n = buffer.readUInt32LE(offset) * 4294967296.0 + low;
  if (low < 0) n += 4294967296;
  return n;
}

function parseBodies(kinectData, data) {
	var offset = 0;

	// body count
	kinectData.bodyCount = data.readUInt16LE(offset); offset += 2;

	// tracking Ids
	for (var s=0; s<6; s++) {
		kinectData.bodyTrackingIds[s] = readInt64LEasFloat(data, offset); offset += 8;
	}

	// parse joint data
	for (var s=0; s<6; s++) {
		for (var j=0; j<25; j++) {
			kinectData.bodies[s][j].x = data.readFloatLE(offset);	offset += 4;
			kinectData.bodies[s][j].y = data.readFloatLE(offset);	offset += 4;
			kinectData.bodies[s][j].z = data.readFloatLE(offset);	offset += 4;
		}
	}
	kinectData.bodiesReady = true;
}

function parseDepth(kinectData, data) {
	var offset = 0;
	//console.log("parsing depth");
	kinectData.depthWidth = data.readUInt16LE(offset); offset += 2;
	kinectData.depthHeight = data.readUInt16LE(offset); offset += 2;

	// copy buffer data to kinectData
	//console.log("parsing depth: " + kinectData.depthWidth + ", " + kinectData.depthHeight);
	data.copy(kinectData.depthBuffer, 0, offset, kinectData.depthWidth * kinectData.depthHeight + offset);
	offset += kinectData.depthWidth * kinectData.depthHeight;
	kinectData.depthReady = true;
}

// maximum size of frame is 247815 bytes (512x424 + 7)
var parseCommandId = -1;
var parseBuffer = new Buffer(247815);
var parseDataOffset = 0;
var parseDataLength = 0;

function parseCommand(data, dataOffset) {
	// parse out main header
	parseCommandId	= data.readUInt8(dataOffset); dataOffset += 1;
	if (parseCommandId != 0 && parseCommandId != 1 )
		throw ("parseCommandId: " + parseCommandId);
	parseDataLength	= data.readUInt32LE(dataOffset); dataOffset += 4;
	if (parseDataLength > 247815)
		throw ("parseDataLength: " + parseDataLength);
	//console.log("parseCommandId: " + parseCommandId + ", parseDataLength: " + parseDataLength);
	return dataOffset;
}

function parseData(data, dataOffset) {
	// keep adding to parseBuffer until we have entire frame
	var dataLength = data.length - dataOffset;
	var copyLength = Math.min(parseDataLength - parseDataOffset, dataLength);
	//console.log("parseDataOffset: " + parseDataOffset + " dataOffset: " + dataOffset + " copyLength: " + copyLength);
	data.copy(parseBuffer, parseDataOffset, dataOffset, dataOffset + copyLength);
	parseDataOffset += copyLength;
	return dataOffset + copyLength;
}

function start(host, port, kinectData) {
	net.createServer(function(socket) {
		console.log('CONNECTED: ' + socket.remoteAddress +':'+ socket.remotePort);
		var remoteAddress = socket.remoteAddress;
		var remotePort = socket.remotePort;

		socket.on('data', function(data) {
			var dataOffset = 0;
			var dataLeft = data.length;

			while (dataLeft > 0) {				
				// if we don't have a command yet, parse it out of data
				if (parseCommandId == -1) {
					try {
						dataOffset = parseCommand(data, dataOffset);
					}
					catch(e) {
						console.log(e);

						// skip this data message as we are unable to parse a command
						parseCommandId = -1; parseDataOffset = 0; parseDataLength = 0;
						dataLeft = 0;
					}
				}

				if (parseCommandId != -1) {
					// parse command's data
					dataOffset = parseData(data, dataOffset);

					// if we are done receiving the buffer, go ahead and parse bodies or depth
					if (parseDataOffset >= parseDataLength-1) {
						//console.log("parseDataOffset: " + parseDataOffset + " parseDataLength: " + parseDataLength);
						if (parseCommandId == 0)
							parseBodies(kinectData, parseBuffer);
						else if (parseCommandId == 1)
							parseDepth(kinectData, parseBuffer);

						// reset command
						parseCommandId = -1; parseDataOffset = 0; parseDataLength = 0;
					}

					// check if we have any data left in packet
					dataLeft -= dataOffset;
					//if (dataLeft > 0)
					//	console.log("data left: " + dataLeft);
				}
			}
		});
		
		socket.on('close', function(data) {
			console.log('CLOSED: ' + remoteAddress +' '+ remotePort);
		});
		
	}).listen(port, host);
	console.log('Server listening on ' + host +':'+ port);
}

exports.start = start;