Microsoft Cube SDK
===
The Microsoft Cube SDK provides the means to collect data from an array of Kinects and display an interactive on the 5 sides of the cube.  Data is collected via the KinectTransport app and then transmitted to Cinder, NodeJS or another framework.  Additionally the DeployTool provides a method for automatically pushing builds from a centralized server to each side of the cube and starting and stopping the application.

## Installing
 1. Clone this repository onto your machine.
 2. Install [NodeJS](http://nodejs.org/) if it hasn't been already installed.
 3. Inside of the [NodeJSSample](Samples/NodeJSSample/) directory run `npm install` to install all required node modules.

## Tools

### Kinect Transport
To run KinectTransport, start the KinectTransport exe.  The app will immediately begin polling the Kinect device and sending data to whatever destination is specified.  Right clicking on the icon in the system tray will bring up options.  A different destination host and whether the app will transport body and depth data can be specified in the options.  Further documentation can be found [here](KinectTransport/README.md).

### Deploy Tool
The DeployTool is designed to allow an executable to be pushed easily from one computer to each of the computers displaying content for the sides of the cube.  Further documentation can be found [here](DeployTool/README.md).

## Samples

### Cinder Sample
To build the Cinder Sample you will need to set your CINDER_DIR environment variable to be the path where you have [Cinder 0.8.6](http://libcinder.org/download/) installed.  Make sure to build both debug and release versions of Cinder and then build the solution in [CinderSample](Samples/CinderSample/) and run the executable created.

### NodeJS Sample
Start server.bat in the [NodeJSSample](Samples/NodeJSSample/) directory and then go to http://localhost:8000 to view the data being sent by the Kinect (KinectTransport must have the destination host specifed to be the machine running the NodeJS server).

### Unity Sample
Still a work in progress (currently only receives and displays body data).  To run: install [Unity](http://unity3d.com/unity/download) and then open the [UnitySample](Samples/UnitySample/) folder in Unity as a project.  The application should display skeleton data if KinectTransport is sending it to that host.

## Further Documentation
* [SDK Architecture](docs/sdk_architecture.md)
* [API Documentation](docs/api_documentation.md)

### CubeSDK Presentation (given at Microsoft during Cube Hackathon)
* [Presentation as Powerpoint](docs/presentation/CubeSDK.pptx)
* [Presentation as PDF](docs/presentation/CubeSDK.pdf)