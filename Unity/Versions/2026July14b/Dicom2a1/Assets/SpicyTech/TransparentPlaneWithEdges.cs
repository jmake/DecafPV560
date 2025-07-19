using UnityEngine;

[RequireComponent(typeof(MeshFilter), typeof(MeshRenderer))]
public class TransparentPlaneWithEdges : MonoBehaviour
{
    public float size = 100f;
    public Color edgeColor = Color.red;
    private LineRenderer lineRenderer;


    void Start()
    {
        MeshFilter mf = GetComponent<MeshFilter>();
        mf.mesh = CreatePlane(size);

        Material mat = new Material(Shader.Find("Custom/TransparentGreen"));
        GetComponent<MeshRenderer>().material = mat;

        lineRenderer = gameObject.AddComponent<LineRenderer>();
        lineRenderer.positionCount = 5;
        lineRenderer.loop = true;
        lineRenderer.useWorldSpace = false;
        lineRenderer.widthMultiplier = 1.0f;
        lineRenderer.material = new Material(Shader.Find("Sprites/Default"));
        lineRenderer.startColor = edgeColor;
        lineRenderer.endColor = edgeColor;

        float h = size / 2f;
        Vector3[] corners = new Vector3[]
        {
            new Vector3(-h, 0, -h),
            new Vector3(h, 0, -h),
            new Vector3(h, 0, h),
            new Vector3(-h, 0, h),
            new Vector3(-h, 0, -h) 
        };

        lineRenderer.SetPositions(corners);
    }

    private Mesh CreatePlane(float size)
    {
        float half = size / 2;

        Mesh mesh = new Mesh();
        mesh.vertices = new Vector3[]
        {
            new Vector3(-half, 0, -half),
            new Vector3(half, 0, -half),
            new Vector3(half, 0, half),
            new Vector3(-half, 0, half)
        };

        mesh.triangles = new int[]
        {
            0, 1, 2,
            0, 2, 3
        };

        mesh.RecalculateNormals();
        return mesh;
    }
}