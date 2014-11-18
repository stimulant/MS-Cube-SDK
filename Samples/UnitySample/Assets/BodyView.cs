using UnityEngine;
using System.Collections;
using System.Collections.Generic;

public class BodyView : MonoBehaviour 
{
	public Material BoneMaterial;
	public GameObject KinectManager;

	private Dictionary<ulong, GameObject> _Bodies = new Dictionary<ulong, GameObject>();
	private Dictionary<int, int> _BoneMap = new Dictionary<int, int>()
	{
		{ 15, 14 },//{ Kinect.JointType.FootLeft, Kinect.JointType.AnkleLeft },
		{ 14, 13 },//{ Kinect.JointType.AnkleLeft, Kinect.JointType.KneeLeft },
		{ 13, 12 },//{ Kinect.JointType.KneeLeft, Kinect.JointType.HipLeft },
		{ 12, 0 }, //{ Kinect.JointType.HipLeft, Kinect.JointType.SpineBase },
		
		{ 19, 18 },//{ Kinect.JointType.FootRight, Kinect.JointType.AnkleRight },
		{ 18, 17 },//{ Kinect.JointType.AnkleRight, Kinect.JointType.KneeRight },
		{ 17, 16 },//{ Kinect.JointType.KneeRight, Kinect.JointType.HipRight },
		{ 16, 0 }, //{ Kinect.JointType.HipRight, Kinect.JointType.SpineBase },
		
		{ 21, 7 }, //{ Kinect.JointType.HandTipLeft, Kinect.JointType.HandLeft },
		{ 22, 7 }, //{ Kinect.JointType.ThumbLeft, Kinect.JointType.HandLeft },
		{ 7, 6 },  //{ Kinect.JointType.HandLeft, Kinect.JointType.WristLeft },
		{ 6, 5 },  //{ Kinect.JointType.WristLeft, Kinect.JointType.ElbowLeft },
		{ 5, 4 },  //{ Kinect.JointType.ElbowLeft, Kinect.JointType.ShoulderLeft },
		{ 4, 20 }, //{ Kinect.JointType.ShoulderLeft, Kinect.JointType.SpineShoulder },
		
		{ 23, 11 },//{ Kinect.JointType.HandTipRight, Kinect.JointType.HandRight },
		{ 24, 11 },//{ Kinect.JointType.ThumbRight, Kinect.JointType.HandRight },
		{ 11, 10 },//{ Kinect.JointType.HandRight, Kinect.JointType.WristRight },
		{ 10, 9 }, //{ Kinect.JointType.WristRight, Kinect.JointType.ElbowRight },
		{ 9, 8 },  //{ Kinect.JointType.ElbowRight, Kinect.JointType.ShoulderRight },
		{ 8, 20 }, //{ Kinect.JointType.ShoulderRight, Kinect.JointType.SpineShoulder },
		
		{ 0, 1 },  //{ Kinect.JointType.SpineBase, Kinect.JointType.SpineMid },
		{ 1, 20 }, //{ Kinect.JointType.SpineMid, Kinect.JointType.SpineShoulder },
		{ 20, 2 }, //{ Kinect.JointType.SpineShoulder, Kinect.JointType.Neck },
		{ 2, 3 },  //{ Kinect.JointType.Neck, Kinect.JointType.Head },
	};
	
	void Update () 
	{
        //Debug.Log("BodyView: Update");
		if (KinectManager == null)
			return;
		KinectAPI kinectAPI = KinectManager.GetComponent<KinectAPI>();
		if (kinectAPI == null)
			return;

		List<ulong> trackedIds = new List<ulong>();
        for (int i = 0; i < 6; i++)
        {
            if (kinectAPI.TrackingIds[i] != 0)
                trackedIds.Add(kinectAPI.TrackingIds[i]);
        }

		// First delete untracked bodies
		List<ulong> knownIds = new List<ulong>(_Bodies.Keys);
		foreach(ulong trackingId in knownIds)
		{
			if(!trackedIds.Contains(trackingId))
			{
				Destroy(_Bodies[trackingId]);
				_Bodies.Remove(trackingId);
			}
		}
		
		for (int i=0; i<6; i++)
		{
            if (kinectAPI.TrackingIds[i] == 0)
                continue;
			if(!_Bodies.ContainsKey(kinectAPI.TrackingIds[i]))
				_Bodies[kinectAPI.TrackingIds[i]] = CreateBodyObject(kinectAPI.TrackingIds[i]);
			RefreshBodyObject(kinectAPI.Bodies[i], _Bodies[kinectAPI.TrackingIds[i]]);
		}
	}
	
	private GameObject CreateBodyObject(ulong id)
	{
		GameObject body = new GameObject("Body:" + id);
		for (int j=0; j<25; j++)
		{
			GameObject jointObj = GameObject.CreatePrimitive(PrimitiveType.Cube);
			
			LineRenderer lr = jointObj.AddComponent<LineRenderer>();
			lr.SetVertexCount(2);
			lr.material = BoneMaterial;
			lr.SetWidth(0.05f, 0.05f);
			
			jointObj.transform.localScale = new Vector3(0.3f, 0.3f, 0.3f);
			jointObj.name = "jt" + j;
			jointObj.transform.parent = body.transform;
		}

		return body;
	}
	
	private void RefreshBodyObject(KinectBody body, GameObject bodyObject)
	{
		for (int j=0; j<25; j++)
		{
			Vector3 sourceJoint = body.Joints[j];
			Vector3? targetJoint = null;

			if(_BoneMap.ContainsKey(j))
			{
				targetJoint = body.Joints[_BoneMap[j]];
			}
			
			Transform jointObj = bodyObject.transform.FindChild("jt" + j);
            //Debug.Log("BodyView: RefreshBodyObject: " + sourceJoint);
			jointObj.localPosition = sourceJoint * 10.0f;
			
			LineRenderer lr = jointObj.GetComponent<LineRenderer>();
			if(targetJoint.HasValue)
			{
				lr.SetPosition(0, jointObj.localPosition);
				lr.SetPosition(1, targetJoint.Value * 10.0f);
				lr.SetColors(Color.red, Color.red);
			}
			else
			{
				lr.enabled = false;
			}
		}
	}
}
