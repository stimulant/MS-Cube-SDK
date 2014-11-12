var socket = io.connect( jQuery.data( $("head")[0], "data").server_url );
$.timeago.settings.strings.seconds = "%d seconds";

//
// DO ACTUAL STARTUP
//
socket.on('connect', function(){});

function blobToImage(imageData) {
  if (Blob && 'undefined' != typeof URL) {
    var blob = new Blob([imageData], {type: 'image/png'});
    return URL.createObjectURL(blob);
  } else if (imageData.base64) {
    return 'data:image/bmp;base64,' + imageData.data;
  } else {
    return 'about:blank';
  }
}

// update depth buffer
var timeSinceUpdateDepth = 0;
socket.on('updateDepth', function(data) {
	//console.log("updating depth");
	var canvas = document.getElementById('depthCanvas');
    var context = canvas.getContext('2d');
    context.clearRect(0, 0, canvas.width, canvas.height);

    var imgArray = new Uint8Array(data.buffer);
    //console.log("updating depth image: " + imgArray.length);
    var imgW = 512, imgH = 424;
    var imgPixels = context.getImageData(0, 0, imgW, imgH);

    var d = 0;
	for(var y = 0; y < imgPixels.height; y++){
	     for(var x = 0; x < imgPixels.width; x++){
	          var i = (y * 4) * imgPixels.width + x * 4;
	          var avg = imgArray[y*imgPixels.width + x];
	          //avg = d%255;

	          imgPixels.data[i] = avg;
	          imgPixels.data[i + 1] = avg;
	          imgPixels.data[i + 2] = avg;
	          imgPixels.data[i + 3] = 255;
	     }
	}
	context.putImageData(imgPixels, 0, 0);
	timeSinceUpdateDepth = 0;
});

// update clients list
var timeSinceUpdateBodies = 0;
socket.on('updateBodies', function(data) {
	//console.log("updating bodies");

	var canvas = document.getElementById('bodyCanvas');
    var context = canvas.getContext('2d');
    context.clearRect(0, 0, canvas.width, canvas.height);
    var radius = 5;

	for (var b in data.bodies) {
		if (data.bodyTrackingIds[b] != 0) {
			for (var i in data.bodies[b]) {
				$('#joint_x_' + i).html(data.bodies[b][i].x);
				$('#joint_y_' + i).html(data.bodies[b][i].y);
				$('#joint_z_' + i).html(data.bodies[b][i].z);

				context.beginPath();
				context.arc(data.bodies[b][i].x * canvas.width/2 + canvas.width/2.0,
							(1.0-data.bodies[b][i].y) * canvas.height/2, radius, 0, 2 * Math.PI, false);
				context.closePath()
				context.fillStyle = 'blue';
				context.fill();
				context.lineWidth = 2;
				context.strokeStyle = '#ffffff';
				context.stroke();
			}
		}
	}

	/*
	//console.log(data.bodyCount);
	if (data.bodyCount > 0)
	{
		console.log("x: " + data.bodies[0][0].x +
					"y: " + data.bodies[0][0].y + 
					"z: " + data.bodies[0][0].z);
	}*/
	timeSinceUpdateBodies = 0;
});

function clearCanvas(canvasName) {
	var canvas = document.getElementById(canvasName);
    var context = canvas.getContext('2d');
    context.clearRect(0, 0, canvas.width, canvas.height);
}

// send updated parameters on an interval
setInterval(
	function() {
		/*
		if (timeSinceUpdateDepth > 0.2)
			clearCanvas('depthCanvas');
		if (timeSinceUpdateBodies > 0.2)
			clearCanvas('bodyCanvas');
		timeSinceUpdateDepth += 1000.0/30.0;
		timeSinceUpdateBodies += 1000.0/30.0;
		*/
	},
1000.0/30.0);