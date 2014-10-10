var net = require('net');
var binary = require('binary');

function start(host, port, kinectData) {

	net.createServer(function(socket) {
		console.log('CONNECTED: ' + socket.remoteAddress +':'+ socket.remotePort);
		var remoteAddress = socket.remoteAddress;
		var remotePort = socket.remotePort;

		socket.on('data', function(data) {
			var skeletonIdx = 0;
			var jointIdx = 0;
			var binaryBuffer = binary()
				.word8lu('command')
				.word16lu('dataLength')
				.buffer('systemTime', 16)
				.word8lu('skeleton1Present')
				.word8lu('skeleton2Present')
				.word8lu('skeleton3Present')
				.word8lu('skeleton4Present')
				.word8lu('skeleton5Present')
				.word8lu('skeleton6Present')
				.word16lu('skeletonCount')
				.tap(function (vars) {
					//console.log('Reading skeletons');
					//console.dir(vars);
					skeletonIdx = 0;
					kinectData.skeletonCount = vars.skeletonCount;
				})
				.loop(function (end1, vars1) {
					// read out skeletons
					if (skeletonIdx >= kinectData.skeletonCount) {
						//console.log("end1");
						end1();
					}

					// read out joints
					jointIdx = 0;
					this.loop(function (end2, vars) {
						//console.log(vars.jointCount);

						if (jointIdx >= 25) {
							//console.log("end2");
							end2();
						}

						// read out joint position
						this
							.word32lu('x')
							.word32lu('y')
							.word32lu('z')
							.tap(function (vars) {
								//console.log("  vars: " + jointIdx);
								kinectData.skeletons[skeletonIdx][jointIdx].x = vars.x;
								kinectData.skeletons[skeletonIdx][jointIdx].y = vars.y;
								kinectData.skeletons[skeletonIdx][jointIdx].z = vars.z;
						});

						jointIdx++;
					});

					skeletonIdx++;
				});

			binaryBuffer.write(data);
		});
		
		socket.on('close', function(data) {
			console.log('CLOSED: ' + remoteAddress +' '+ remotePort);
		});
		
	}).listen(port, host);
	console.log('TCP Server listening on ' + host +':'+ port);
}

exports.start = start;