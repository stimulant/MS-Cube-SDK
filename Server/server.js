var tcpServer = require('./lib/tcpServer.js');
var webServer = require('./lib/webServer.js');
var kinectData = {};

tcpServer.start('127.0.0.1', 3000, kinectData);
webServer.start('127.0.0.1', 8000, kinectData);
