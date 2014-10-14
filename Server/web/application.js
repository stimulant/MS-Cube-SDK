var socket = io.connect( jQuery.data( $("head")[0], "data").server_url );
$.timeago.settings.strings.seconds = "%d seconds";

//
// DO ACTUAL STARTUP
//
socket.on('connect', function(){});

// update clients list
socket.on('updateData', function(data) {
	//console.log(data.skeletonCount);

	var canvas = document.getElementById('skeletonCanvas');
    var context = canvas.getContext('2d');
    context.clearRect(0, 0, canvas.width, canvas.height);
    var radius = 5;

	if (data.skeletonCount > 0) {
		console.dir(data.skeletons[0][0]);
		for (var i in data.skeletons[0]) {

			$('#joint_x_' + i).html(data.skeletons[0][i].x);
			$('#joint_y_' + i).html(data.skeletons[0][i].y);
			$('#joint_z_' + i).html(data.skeletons[0][i].z);


			context.beginPath();
			context.arc(data.skeletons[0][i].x * canvas.width/2 + canvas.width/2.0,
						(1.0-data.skeletons[0][i].y) * canvas.height/2, radius, 0, 2 * Math.PI, false);
			context.closePath()
			context.fillStyle = 'blue';
			context.fill();
			context.lineWidth = 2;
			context.strokeStyle = '#ffffff';
			context.stroke();
		}
	}

	/*
	//console.log(data.skeletonCount);
	if (data.skeletonCount > 0)
	{
		console.log("x: " + data.skeletons[0][0].x +
					"y: " + data.skeletons[0][0].y + 
					"z: " + data.skeletons[0][0].z);
	}*/
});

// send updated parameters on an interval
setInterval(
	function() {
	},
1000.0/30.0);