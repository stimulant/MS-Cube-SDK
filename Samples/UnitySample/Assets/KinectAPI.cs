using UnityEngine;
using System;
using System.IO;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Collections;
using System.Collections.Generic;

public class KinectBody
{
    public IList<Vector3> Joints = new List<Vector3>();
}

public class KinectAPI : MonoBehaviour
{
    public int BodyCount = 0;
    public UInt64[] TrackingIds = new UInt64[6];
    public IList<KinectBody> Bodies = new List<KinectBody>();
    public byte[] DepthData = new byte[512 * 424];

    TcpListener server = null;
    TcpClient client;
    bool connected = false;
    Byte[] bytes;
    int command;

    // Use this for initialization
    void Start()
    {
        Debug.Log("KinectAPI: Start");

        // Set the TcpListener on port 13000.
        Int32 port = 3000;
        IPAddress localAddr = IPAddress.Parse("0.0.0.0");

        // TcpListener server = new TcpListener(port);
        server = new TcpListener(localAddr, port);

        // Start listening for client requests.
        server.Start();

        // Buffer for reading data
        bytes = new Byte[250000];

        command = -1;
    }

	int parseCommandId = -1;
	MemoryStream parseStream = new MemoryStream(247815);
	int parseDataOffset = 0;
	int parseDataLength = 0;

	void ResetParsing()
	{
		parseCommandId = -1;
		parseStream = new MemoryStream (247815);
		parseDataOffset = 0;
		parseDataLength = 0;
	}

	void ParseCommand(BinaryReader reader)
	{
		parseCommandId = (int)reader.ReadByte();
		parseDataLength = reader.ReadInt32();
		//Debug.Log("KinectAPI: Command: " + parseCommandId + " length:" + parseDataLength);
	}

	void ParseBodies()
	{
		Debug.Log("KinectAPI: Parsing bodies " + parseStream.Length);
		using (BinaryReader reader = new BinaryReader(parseStream, Encoding.Default))
		{
			// parse bodies
			Bodies.Clear();
			BodyCount = reader.ReadUInt16();
			for (int i = 0; i < 6; i++)
				TrackingIds[i] = reader.ReadUInt64();
			
			for (int b = 0; b < 6; b++)
			{
				KinectBody body = new KinectBody();
				for (int j = 0; j < 25; j++)
				{
					body.Joints.Add(
						new Vector3(
						reader.ReadSingle(),
						reader.ReadSingle(),
						reader.ReadSingle()));
				}
				Bodies.Add(body);
			}
		}
	}

	void ParseDepth()
	{
		using (BinaryReader reader = new BinaryReader(parseStream, Encoding.Default))
		{
			// parse depth
			//Debug.Log("KinectAPI: getting depth");
			int width = reader.ReadUInt16();
			int height = reader.ReadUInt16();
			DepthData = reader.ReadBytes(512 * 424);
		}
	}

    // Update is called once per frame
    void Update()
    {
        // accept a connection if we have one pending
        if (!connected && server.Pending())
        {
            Debug.Log("KinectAPI: Connecting... ");
            // Perform a blocking call to accept requests. 
            // You could also user server.AcceptSocket() here.
            client = server.AcceptTcpClient();
            Debug.Log("KinectAPI: Connected!");
            connected = true;

			// reset parse state
			ResetParsing();
        }

        if (connected)
        {
            // Get a stream object for reading and writing
            NetworkStream stream = client.GetStream();

            // Loop to receive all the data sent by the client. 
            int dataLeft = stream.Read(bytes, 0, bytes.Length);
			if (dataLeft != 0)
            {
                // parse out command and length
                using (MemoryStream ms = new MemoryStream(bytes))
                {
					using (BinaryReader reader = new BinaryReader(ms, Encoding.Default))
					{
						while (dataLeft > 0)
						{                    
							// parse out command if we don't have one
							if (parseCommandId == -1)
							{
								try
								{
									ParseCommand(reader);
									dataLeft -= 5;
								}
								catch(Exception e)
								{
									Debug.LogException(e);
								}
							}

							if (parseCommandId != -1 && dataLeft > 0)
							{
								// write bytes to our parsestream
								//Debug.Log("KinectAPI: Copying data");
								var copyLength = Math.Min(parseDataLength - parseDataOffset, dataLeft);
								parseStream.Write(reader.ReadBytes(copyLength), 0, copyLength);
								dataLeft -= copyLength;
								parseDataOffset += copyLength;

								if (parseDataOffset >= parseDataLength-1) 
								{
									parseStream.Position = 0;
									//Debug.Log("KinectAPI: do command: " + parseCommandId);
									if (parseCommandId == 0)
										ParseBodies();
									else if (parseCommandId == 1)
										ParseDepth();

									ResetParsing();
								}
							}
	                    }
	                }
				}
            }
            else
            {
                // Shutdown and end connection
                client.Close();
                connected = false;
            }
        }
    }
}