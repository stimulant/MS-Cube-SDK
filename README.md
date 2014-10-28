Microsoft Cube SDK
===

The Microsoft Cube SDK provides the means to collect data from an array of Kinects and display an interactive on the 5 sides of the cube.  Data is collected via the KinectTrasport app and then transmitted to Cinder, NodeJS or another framework.

# Installing

 1. Clone this repository onto your machine.
 2. Install [NodeJS](http://nodejs.org/) if it hasn't been already installed.
 3. Inside of the [Server](Server/) directory run `npm install` to install all required node modules.

# Kinect Transport

To run KinectTransport, start the KinectTransport exe.  The app will immediately begin polling the Kinect device and sending data to whatever destination is specified.  Right clicking on the icon in the system tray will bring up options.  A different destination host and whether the app will transport skeleton and depth data can be specified in the options.  Destination host must be an IP address (for now).

# NodeJS Data visualization

Start server.bat in the [Server](Server/) directory and then go to http://localhost:8000 to view the data being sent by the Kinect (KinectTransport must have the destination host specifed to be the machine running the NodeJS server).

# Further Documentation

* [SDK Architecture](docs/sdk_architecture.md)
* [API Documentation](docs/api_documentation.md)