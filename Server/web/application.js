var socket = io.connect( jQuery.data( $("head")[0], "data").server_url );
$.timeago.settings.strings.seconds = "%d seconds";

//
// DO ACTUAL STARTUP
//
socket.on('connect', function(){});

// update clients list
socket.on('updateclients', function(data) { updatestatus(data); });

// send updated parameters on an interval
setInterval(
	function() {
	},
1000.0/30.0);