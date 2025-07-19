using UnityEngine;

public class WireframeCube : MonoBehaviour
{
    [Header("Ranges: x = [-1,0] to [0,1]")]
    public Vector2 xRange = new Vector2(-0.5f, 0.5f);
    public Vector2 yRange = new Vector2(-0.5f, 0.5f);
    public Vector2 zRange = new Vector2(-0.5f, 0.5f);

    public float lineWidth = 0.02f;

    private LineRenderer lineRenderer;

    void Start()
    {
        SetupLineRenderer();
        SetWireframeCube();
    }

    void Update()
    {
        SetWireframeCube(); // update cube positions every frame
    }

    void OnValidate()
    {
        xRange.x = Mathf.Clamp(xRange.x, -1f, 0f);
        xRange.y = Mathf.Clamp(xRange.y, 0f, 1f);

        yRange.x = Mathf.Clamp(yRange.x, -1f, 0f);
        yRange.y = Mathf.Clamp(yRange.y, 0f, 1f);

        zRange.x = Mathf.Clamp(zRange.x, -1f, 0f);
        zRange.y = Mathf.Clamp(zRange.y, 0f, 1f);
    }

    void SetupLineRenderer()
    {
        lineRenderer = GetComponent<LineRenderer>();
        if (lineRenderer == null)
            lineRenderer = gameObject.AddComponent<LineRenderer>();

        lineRenderer.useWorldSpace = false;
        lineRenderer.loop = false;

        lineRenderer.startWidth = lineWidth;
        lineRenderer.endWidth = lineWidth;

        if (lineRenderer.sharedMaterial == null)
        {
            Material defaultMat = new Material(Shader.Find("Sprites/Default"));
            defaultMat.color = Color.green;
            lineRenderer.material = defaultMat;
        }
    }

    void SetWireframeCube()
    {
        float xmin = xRange.x;
        float xmax = xRange.y;
        float ymin = yRange.x;
        float ymax = yRange.y;
        float zmin = zRange.x;
        float zmax = zRange.y;

        Vector3[] points = new Vector3[]
        {
            new Vector3(xmin, ymin, zmin),
            new Vector3(xmax, ymin, zmin),
            new Vector3(xmax, ymin, zmax),
            new Vector3(xmin, ymin, zmax),
            new Vector3(xmin, ymin, zmin),

            new Vector3(xmin, ymax, zmin),
            new Vector3(xmax, ymax, zmin),
            new Vector3(xmax, ymin, zmin),
            new Vector3(xmax, ymax, zmin),
            new Vector3(xmax, ymax, zmax),
            new Vector3(xmax, ymin, zmax),
            new Vector3(xmax, ymax, zmax),
            new Vector3(xmin, ymax, zmax),
            new Vector3(xmin, ymin, zmax),
            new Vector3(xmin, ymax, zmax),
            new Vector3(xmin, ymax, zmin),

            new Vector3(xmin, ymax, zmin),
            new Vector3(xmax, ymax, zmin),
            new Vector3(xmax, ymax, zmax),
            new Vector3(xmin, ymax, zmax),
            new Vector3(xmin, ymax, zmin)
        };

        lineRenderer.positionCount = points.Length;
        lineRenderer.SetPositions(points);
    }
}