using UnityEngine;

public class CameraTextureToPlane : MonoBehaviour
{
    public Camera cameraToControl;
    public int BitsInDepth = 24;
    public Vector3 planePosition = new Vector3(0, 0, 100);
    public Vector2 planeScale = new Vector2(1, 1);

    private RenderTexture renderTexture;
    private GameObject displayPlane;
    private int width, height;

    void Start()
    {
        CreateCamera();
        InitializeTextures();
        CreateDisplayPlane();
    }

    void LateUpdate()
    {
        if (cameraToControl != null)
            cameraToControl.Render();
    }

    void CreateCamera()
    {
        if (cameraToControl) return;

        GameObject camObj = new GameObject("Camera2Texture");
        camObj.transform.position = new Vector3(0, 0, -10); // Set an initial position
        Camera cam = camObj.AddComponent<Camera>();
        cam.CopyFrom(Camera.main);
        cameraToControl = cam;
    }

    void InitializeTextures()
    {
        width = Screen.width;
        height = Screen.height;

        renderTexture = new RenderTexture(width, height, BitsInDepth);
        renderTexture.Create();

        cameraToControl.targetTexture = renderTexture;
    }

    void CreateDisplayPlane()
    {
        displayPlane = GameObject.CreatePrimitive(PrimitiveType.Plane);
        displayPlane.name = "RenderPlane";
        displayPlane.transform.position = planePosition;
        displayPlane.transform.localScale = new Vector3(planeScale.x, 1, planeScale.y);

        // Create material
        Material mat = new Material(Shader.Find("Unlit/Texture"));
        mat.mainTexture = renderTexture;

        displayPlane.GetComponent<Renderer>().material = mat;
    }
}
