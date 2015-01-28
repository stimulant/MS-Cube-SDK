API Documentation
======================

The main part of the SDK is the API specification that will define how Kinect data is sent from the Kinect Transport application to either the Node.js Server or the Client application. In addition, this API is the same format that the Node.js Server uses to re-send Kinect data (this will allow the same Client application to receive data from the Transport application or the Server).

# Specification:

A Kinect API stream consists of a sequence of messages. Each message has a 4-byte header followed by a variable-length data block.

| Byte   | Description
|--------|----------------------------
|0       | Command code that determines the format of the data
|1-2     | Length of data block from 0 to 65535 sent as unsigned 2-byte number
|n bytes | The data payload for the command

## Commands

### Body Update (command 0)

The Body Update command sends a list of the bodies that the Kinect detects and all of their joint positions and orientations along with hand states for each of the bodies hands.
Header that is present no matter how many bodies are present.

| Byte   | Description
|--------|----------------------------
|0-8     | Timestamp
|9-56    | Unsigned 64 bit integer ID for each body that is tracked, 0 if this body is not tracked.

After this there is a repeating section for each body that indicates positions of all joints:

| Byte   | Description
|--------|----------------------------
|0-11    | JointType_SpineBase XYZ position
|12-23   | JointType_SpineMid XYZ position
|24-35   | JointType_Neck XYZ position
|36-47   | JointType_Head XYZ position
|48-59   | JointType_ShoulderLeft XYZ position
|60-71   | JointType_ElbowLeft XYZ position
|72-83   | JointType_WristLeft XYZ position
|84-95   | JointType_HandLeft XYZ position
|96-107  | JointType_ShoulderRight XYZ position
|108-119 | JointType_ElbowRight XYZ position
|120-131 | JointType_WristRight XYZ position
|132-143 | JointType_HandRight XYZ position
|144-155 | JointType_HipLeft XYZ position
|156-167 | JointType_KneeLeft XYZ position
|168-179 | JointType_AnkleLeft XYZ position
|180-191 | JointType_FootLeft XYZ position
|192-203 | JointType_HipRight XYZ position
|204-215 | JointType_KneeRight XYZ position
|216-227 | JointType_AnkleRight XYZ position
|228-139 | JointType_FootRight XYZ position
|140-151 | JointType_SpineShoulder XYZ position
|152-163 | JointType_HandTipLeft XYZ position
|164-175 | JointType_ThumbLeft XYZ position
|176-187 | JointType_HandTipRight XYZ position
|188-199 | JointType_ThumbRight XYZ position

After this there is a repeating section for each body that indicates state for left and right hands:

| Byte   | Description
|--------|----------------------------
|0-1     | Left Hand state
|1-2     | Right Hand state

States are encoded as byte values as follows:

| Value  | State
|--------|----------------------------
|0       | Unknown
|1       | NotTracked
|2       | Open
|3       | Closed
|4       | Lasso

After this there is a repeating section for each body that indicates orientations of all joints as four unit quaternions:

| Byte   | Description
|--------|----------------------------
|0-15    | JointType_SpineBase XYZW orientation
|16-31   | JointType_SpineMid XYZW orientation
|32-47   | JointType_Neck XYZW orientation
|48-63   | JointType_Head XYZW orientation
|64-79   | JointType_ShoulderLeft XYZW orientation
|80-95   | JointType_ElbowLeft XYZW orientation
|96-111  | JointType_WristLeft XYZW orientation
|112-127 | JointType_HandLeft XYZW orientation
|128-143 | JointType_ShoulderRight XYZW orientation
|144-159 | JointType_ElbowRight XYZW orientation
|160-175 | JointType_WristRight XYZW orientation
|176-191 | JointType_HandRight XYZW orientation
|192-207 | JointType_HipLeft XYZW orientation
|208-223 | JointType_KneeLeft XYZW orientation
|224-239 | JointType_AnkleLeft XYZW orientation
|240-255 | JointType_FootLeft XYZW orientation
|256-271 | JointType_HipRight XYZW orientation
|272-287 | JointType_KneeRight XYZW orientation
|288-303 | JointType_AnkleRight XYZW orientation
|304-319 | JointType_FootRight XYZW orientation
|320-335 | JointType_SpineShoulder XYZW orientation
|336-351 | JointType_HandTipLeft XYZW orientation
|352-367 | JointType_ThumbLeft XYZW orientation
|368-383 | JointType_HandTipRight XYZW orientation
|384-399 | JointType_ThumbRight XYZW orientation

### Depth Update (command 1)

The Depth Update command sends the current depth buffer from the Kinect.

| Byte   | Description
|--------|----------------------------
|0-1     | Width of depth buffer
|2-3     | Height of depth buffer
|4-247815| Depth frame buffer (8 bytes per pixel)
