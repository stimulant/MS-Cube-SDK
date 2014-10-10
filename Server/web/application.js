var socket = io.connect( jQuery.data( $("head")[0], "data").server_url );
$.timeago.settings.strings.seconds = "%d seconds";

//
// DO ACTUAL STARTUP
//
socket.on('connect', function(){});

// update clients list
socket.on('updateData', function(data) {
	//console.log(data.skeletonCount);
	if (data.skeletonCount > 0)
	{
		console.log("x: " + data.skeletons[0][0].x +
					"y: " + data.skeletons[0][0].y + 
					"z: " + data.skeletons[0][0].z);
	}
});

// send updated parameters on an interval
setInterval(
	function() {
	},
1000.0/30.0);