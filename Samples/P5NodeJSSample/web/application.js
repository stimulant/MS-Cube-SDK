var socket = io.connect( jQuery.data( $("head")[0], "data").server_url );
$.timeago.settings.strings.seconds = "%d seconds";

//
// DO ACTUAL STARTUP
//
socket.on('connect', function(){});

// update depth buffer
var timeSinceUpdateDepth = 0;
socket.on('updateDepth', function(data) {
	var imgArray = new Uint8Array(data.buffer);
	updateDepth(imgArray);
});

// update clients list
var timeSinceUpdateBodies = 0;
socket.on('updateBodies', function(data) {
	updateBodies(data);
});