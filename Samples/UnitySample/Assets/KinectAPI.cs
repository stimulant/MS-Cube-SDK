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
        }

        if (connected)
        {
            // Get a stream object for reading and writing
            NetworkStream stream = client.GetStream();

            // Loop to receive all the data sent by the client. 
            int bytesRead = stream.Read(bytes, 0, bytes.Length);
            if (bytesRead != 0)
            {
                // parse out command and length
                using (MemoryStream ms = new MemoryStream(bytes))
                {
                    /*you can change Encoding.Default if bytes is in other Encoding format!*/
                    using (BinaryReader reader = new BinaryReader(ms, Encoding.Default))
                    {
                        int command = (int)reader.ReadByte();
                        UInt32 length = reader.ReadUInt32();
                        Debug.Log("KinectAPI: Command: " + command + " length:" + length + " bytes read: " + bytesRead);

                        // parse commands
                        if (command == 0)
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
                        else if (command == 1)
                        {
                            // parse depth
                            Debug.Log("KinectAPI: getting depth");
                            int width = reader.ReadUInt16();
                            int height = reader.ReadUInt16();
                            DepthData = reader.ReadBytes(512 * 424);
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