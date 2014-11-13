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

The Body Update command sends a list of the bodies that the Kinect detects and all of their joint positions.
Header that is present no matter how many bodies are present.

| Byte   | Description
|--------|----------------------------
|0-8     | Timestamp
|9-56    | Unsigned 64 bit integer ID for each body that is tracked, 0 if this body is not tracked.

After this there is a repeating section for each body that indicates positions of all joints:

| Byte   | Description
|--------|----------------------------
|0 		 | Skeleton index
|1-12    | JointType_SpineBase XYZ position
|13-24   | JointType_SpineMid XYZ position
|25-36   | JointType_Neck XYZ position
|37-48   | JointType_Head XYZ position
|49-60   | JointType_ShoulderLeft XYZ position
|61-72   | JointType_ElbowLeft XYZ position
|73-84   | JointType_WristLeft XYZ position
|85-96   | JointType_HandLeft XYZ position
|97-108  | JointType_ShoulderRight XYZ position
|109-120 | JointType_ElbowRight XYZ position
|121-132 | JointType_WristRight XYZ position
|133-144 | JointType_HandRight XYZ position
|145-156 | JointType_HipLeft XYZ position
|157-168 | JointType_KneeLeft XYZ position
|169-180 | JointType_AnkleLeft XYZ position
|181-192 | JointType_FootLeft XYZ position
|193-204 | JointType_HipRight XYZ position
|205-216 | JointType_KneeRight XYZ position
|217-228 | JointType_AnkleRight XYZ position
|229-140 | JointType_FootRight XYZ position
|141-152 | JointType_SpineShoulder XYZ position
|153-164 | JointType_HandTipLeft XYZ position
|165-176 | JointType_ThumbLeft XYZ position
|177-188 | JointType_HandTipRight XYZ position
|189-200 | JointType_ThumbRight XYZ position


### Depth Update (command 1)

The Depth Update command sends the current depth buffer from the Kinect.

| Byte   | Description
|--------|----------------------------
|0-1     | Width of depth buffer
|2-3     | Height of depth buffer
|4-247815| Depth frame buffer (8 bytes per pixel)
