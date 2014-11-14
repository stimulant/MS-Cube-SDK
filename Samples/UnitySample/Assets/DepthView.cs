using UnityEngine;
using System.Collections;

public class DepthSourceView : MonoBehaviour
{
	public GameObject KinectManager;
	
	private Mesh _Mesh;
	private Vector3[] _Vertices;
	private Vector2[] _UV;
	private int[] _Triangles;
	
	// Only works at 4 right now
	private const int _DepthWidth = 512;
	private const int _DepthHeight = 424;
	private const int _DownsampleSize = 4;
	private const double _DepthScale = 0.1f;
	private const int _Speed = 50;
	
	void Start()
	{
		// Create mesh
		CreateMesh(_DepthWidth / _DownsampleSize, _DepthHeight / _DownsampleSize);
	}
	
	void CreateMesh(int width, int height)
	{
		_Mesh = new Mesh();
		GetComponent<MeshFilter>().mesh = _Mesh;
		
		_Vertices = new Vector3[width * height];
		_UV = new Vector2[width * height];
		_Triangles = new int[6 * ((width - 1) * (height - 1))];
		
		int triangleIndex = 0;
		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				int index = (y * width) + x;
				
				_Vertices[index] = new Vector3(x, -y, 0);
				_UV[index] = new Vector2(((float)x / (float)width), ((float)y / (float)height));
				
				// Skip the last row/col
				if (x != (width - 1) && y != (height - 1))
				{
					int topLeft = index;
					int topRight = topLeft + 1;
					int bottomLeft = topLeft + width;
					int bottomRight = bottomLeft + 1;
					
					_Triangles[triangleIndex++] = topLeft;
					_Triangles[triangleIndex++] = topRight;
					_Triangles[triangleIndex++] = bottomLeft;
					_Triangles[triangleIndex++] = bottomLeft;
					_Triangles[triangleIndex++] = topRight;
					_Triangles[triangleIndex++] = bottomRight;
				}
			}
		}
		
		_Mesh.vertices = _Vertices;
		_Mesh.uv = _UV;
		_Mesh.triangles = _Triangles;
		_Mesh.RecalculateNormals();
	}
	
	void Update()
	{
		Debug.Log("DepthView: Update");
		if (KinectManager == null)
			return;
		KinectAPI kinectAPI = KinectManager.GetComponent<KinectAPI>();
		if (kinectAPI == null)
			return;
		
		float yVal = Input.GetAxis("Horizontal");
		float xVal = -Input.GetAxis("Vertical");
		
		transform.Rotate(
			(xVal * Time.deltaTime * _Speed),
			(yVal * Time.deltaTime * _Speed),
			0,
			Space.Self);

		RefreshData(kinectAPI.DepthData, _DepthWidth, _DepthHeight);
	}
	
	private void RefreshData(byte[] depthData, int colorWidth, int colorHeight)
	{
		/*
		var frameDesc = _Sensor.DepthFrameSource.FrameDescription;
		
		ColorSpacePoint[] colorSpace = new ColorSpacePoint[depthData.Length];
		_Mapper.MapDepthFrameToColorSpace(depthData, colorSpace);
		
		for (int y = 0; y < frameDesc.Height; y += _DownsampleSize)
		{
			for (int x = 0; x < frameDesc.Width; x += _DownsampleSize)
			{
				int indexX = x / _DownsampleSize;
				int indexY = y / _DownsampleSize;
				int smallIndex = (indexY * (frameDesc.Width / _DownsampleSize)) + indexX;
				
				double avg = GetAvg(depthData, x, y, frameDesc.Width, frameDesc.Height);
				
				avg = avg * _DepthScale;
				
				_Vertices[smallIndex].z = (float)avg;
				
				// Update UV mapping with CDRP
				var colorSpacePoint = colorSpace[(y * frameDesc.Width) + x];
				_UV[smallIndex] = new Vector2(colorSpacePoint.X / colorWidth, colorSpacePoint.Y / colorHeight);
			}
		}
		
		_Mesh.vertices = _Vertices;
		_Mesh.uv = _UV;
		_Mesh.triangles = _Triangles;
		_Mesh.RecalculateNormals();
		*/
	}
	
	private double GetAvg(byte[] depthData, int x, int y, int width, int height)
	{
		double sum = 0.0;
		for (int y1 = y; y1 < y + 4; y1++)
		{
			for (int x1 = x; x1 < x + 4; x1++)
			{
				int fullIndex = (y1 * width) + x1;
				
				if (depthData[fullIndex] == 0)
					sum += 4500;
				else
					sum += depthData[fullIndex];
				
			}
		}
		
		return sum / 16;
	}
}
