using UnityEngine;
using UnityEngine.UI;


public class CameraTextureCapturer : MonoBehaviour
{
    public Camera cameraToControl;
    public int BitsInDepth = 24;

    private Texture2D texture2D;
    private RenderTexture renderTexture;
    private RawImage rawImage;
    private int width, height;


    void Start()
    {
        CreateCamera(); 
        SetupCanvasAndRawImage();
        InitializeTextures();
    }

    void LateUpdate()
    {
        UpdateCameraTexture();
    }


    void CreateCamera() 
    {
        if (cameraToControl) return; 

        GameObject camObj = new GameObject("Camera2Texture");
        Camera cam = camObj.AddComponent<Camera>();
        gameObject.transform.SetParent( camObj.transform ); // Set as child 

        cam.CopyFrom(Camera.main);
        cameraToControl = cam; 
    }

    void SetupCanvasAndRawImage()
    {
        // Create Canvas
        GameObject canvasGO = new GameObject("AutoCanvas");
        canvasGO.layer = LayerMask.NameToLayer("UI");
        Canvas canvas = canvasGO.AddComponent<Canvas>();
        canvas.renderMode = RenderMode.ScreenSpaceOverlay;
        canvasGO.AddComponent<CanvasScaler>();
        canvasGO.AddComponent<GraphicRaycaster>();

        // Create RawImage
        GameObject imageGO = new GameObject("CapturedImage");
        imageGO.transform.SetParent(canvasGO.transform);
        rawImage = imageGO.AddComponent<RawImage>();

        RectTransform rt = rawImage.rectTransform;
        rt.anchorMin = Vector2.zero;
        rt.anchorMax = Vector2.one;
        rt.offsetMin = Vector2.zero;
        rt.offsetMax = Vector2.zero;
    }

    void InitializeTextures()
    {
        width = Screen.width;
        height = Screen.height;

        renderTexture = new RenderTexture(width, height, BitsInDepth);
        texture2D = new Texture2D(width, height, TextureFormat.RGB24, false);

        // If camera = camera.Main then 
        // Unity stops rendering that camera to the screen (i.e., Game View), and 
        // instead it only renders to the RenderTexture.
        if (cameraToControl != null)
            cameraToControl.targetTexture = renderTexture;
    }

    void UpdateCameraTexture()
    {
        if (cameraToControl == null || renderTexture == null || texture2D == null)
            return;

        RenderTexture.active = renderTexture;
        texture2D.ReadPixels(new Rect(0, 0, width, height), 0, 0);
        texture2D.Apply();
        RenderTexture.active = null;

        rawImage.texture = texture2D;
    }
}
