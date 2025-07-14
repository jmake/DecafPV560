using UnityEngine;
using UnityEngine.UI;

using SpicyTech; 


namespace SpicyTech {


public class Camera2Image : MainPanel
{

    public Camera cameraToControl = null;

    int width2; 
    int hight2; 
    int BitsInDepth;

    Texture2D texture2D = null; 
    RenderTexture renderTexture = null; 


    void Start()
    {
        base.Init(); 
        if(!cameraToControl) cameraToControl = CopyCameraMain("Camera2Image"); 
        gameObject.transform.SetParent( cameraToControl.transform ); // Set as child 

        width2 = (int)(Screen.width);
        hight2 = (int)(Screen.height);
        BitsInDepth = 24; 

        Camera2TextureStart(); 
    }


    void Update() 
    {
        base.Apply(); 
        Camera2TextureUpdate(); 
    }


    void Camera2TextureStart() 
    {
        if (texture2D != null)  UnityEngine.GameObject.Destroy(texture2D);
        if (renderTexture != null)  UnityEngine.GameObject.Destroy(renderTexture);

        texture2D = new Texture2D(width2, hight2, TextureFormat.RGB24, false);
        renderTexture = new RenderTexture(width2, hight2, BitsInDepth);

        // visible only 'Touched' layer
        //cameraToControl.cullingMask &= (1 << LayerMask.NameToLayer("Touched"));
    }


    void Camera2TextureUpdate() 
    {
        GameObject oir;  
        UnityEngine.UI.RawImage image;

        cameraToControl.targetTexture = renderTexture;

        oir = base.backgroundObject;
        image = oir.GetComponent<UnityEngine.UI.RawImage>();  
        image.texture = texture2D;

        RenderTexture.active = renderTexture;
        
        texture2D.ReadPixels(new Rect(0, 0, width2, hight2), 0, 0);
        texture2D.Apply();

        RenderTexture.active = null;
    }

} 



public class MainPanel : CameraTools
{
    [Header("Normalized Position and Size")]
    
    [Range(0.0f, 1.0f)]
    public float xmin = 0.01f;

    [Range(0.0f, 1.0f)]
    public float ymin = 0.02f;

    [Range(0.0f, 1.0f)]
    public float width = 1f - 0.02f;

    [Range(0.0f, 1.0f)]
    public float hight = 1f - 0.04f;

    [HideInInspector]
    public GameObject backgroundObject; 

    GameObject mainPanelObject;
    GameObject mainCanvasObject;

    Vector4 previousParams;


    public void Init()
    {
        backgroundObject = InitUI();
    }


    public void Apply()
    {
        Vector4 currentParams = new Vector4(xmin, ymin, width, hight);
        if (currentParams != previousParams)
        {
            RectTransformCreate(mainPanelObject, currentParams);
            previousParams = currentParams;
        }
    }


    public GameObject InitUI()
    {
        mainCanvasObject = CreateCanvas("CanvasUI");
        mainPanelObject = CreatePanel(mainCanvasObject, "MainPanel");

        RectTransformCreate(mainPanelObject, new Vector4(xmin, ymin, width, hight));
        previousParams = new Vector4(xmin, ymin, width, hight);

        Color color = new Color(1.0f, 1.0f, 1.0f, 1.0f);
        Vector4 parameters = new Vector4(0.05f,0.05f, 0.9f,0.9f);
        return BackGroundImageCreate(mainPanelObject, "rawImage", color, parameters); 
    }
}



public class CameraTools : MonoBehaviour
{

    public 
    Camera CopyCameraMain(string name) 
    {
        GameObject camObj = new GameObject(name);

        Camera cam = camObj.AddComponent<Camera>();
        //gameObject.transform.SetParent( camObj.transform ); // Set as child 

        cam.CopyFrom(Camera.main);
        //cameraToControl = cam; 
        return cam; 
    }


    public 
    GameObject BackGroundImageCreate(GameObject o, string name, Color color, Vector4 parameters)
    {
        GameObject obj = new GameObject(name);
        obj.transform.SetParent(o.transform);

        RectTransformCreate(obj, parameters); 

        UnityEngine.UI.RawImage backgroundImage = obj.AddComponent<UnityEngine.UI.RawImage>();
        backgroundImage.color = color;   
        return obj; 
    }


    public RectTransform RectTransformCreate(GameObject obj, Vector4 param)
    {
        RectTransform rect;  
        
        rect = obj.GetComponent<RectTransform>();
        if (rect == null) rect = obj.AddComponent<RectTransform>();

        Vector2 size = new Vector2(param.z, param.w);
        Vector2 position = new Vector2(param.x, param.y);

        float xmin1 = 1f - position.x;
        float ymin1 = 1f - position.y;
        Vector2 min = new Vector2(1f, 1f) - new Vector2(xmin1, ymin1);
        Vector2 max = min + size;

        rect.offsetMin = Vector2.zero;
        rect.offsetMax = Vector2.zero;
        rect.anchorMin = min;
        rect.anchorMax = max;
        return rect; 
    }

    public  GameObject CreateCanvas(string name)
    {
        GameObject obj = new GameObject(name);
        Canvas canvas = obj.AddComponent<Canvas>();
        canvas.renderMode = RenderMode.ScreenSpaceOverlay;

        CanvasScaler scaler = obj.AddComponent<CanvasScaler>();
        scaler.uiScaleMode = CanvasScaler.ScaleMode.ScaleWithScreenSize;

        obj.AddComponent<GraphicRaycaster>();
        return obj;
    }

    public GameObject CreatePanel(GameObject parent, string name)
    {
        GameObject panel = new GameObject(name);
        panel.transform.SetParent(parent.transform);

        RectTransform rect;          
        rect = panel.GetComponent<RectTransform>();
        if (rect == null) rect = panel.AddComponent<RectTransform>();

        Image img = panel.AddComponent<Image>();
        img.color = new Color(1, 1, 1, 0.5f);

        return panel;
    }
}

/*
public class CameraTools : MonoBehaviour
{
    //
    //            <-  width = 1 ->     
    //             --------------     ^
    //            |              |    | 
    //            |              |  hight = 1
    //            |              |    |
    //             --------------     v
    //  (xmin,ymin) = (0,0)
    //
    public float xmin = 0.1f; 
    public float ymin = 0.9f;  

    public float width = 0.75f; 
    public float hight = 0.50f; 

    GameObject mainCanvasObject; 



    void Update()
    {
        Clear(); 
        UILevel1(); 
    }


    GameObject UILevel1() 
    {
        Vector2 size = new Vector2(width, hight); 
        Vector2 position = new Vector2(xmin, ymin); //ymin1-hight1);

        mainCanvasObject = CanvasObjectCreate("canvasName"); 
        GameObject mainPanel = PanelCreate(mainCanvasObject, "UILevel1", position, size); 
        return mainPanel; 
    }


    void Clear()
    {
        if (mainCanvasObject != null) 
        {
            UnityEngine.Object.Destroy(mainCanvasObject);
            mainCanvasObject = null;
        }
    }


    public 
    GameObject CanvasObjectCreate(string name)
    {
        GameObject obj = new GameObject(name);

        Canvas canvas = obj.AddComponent<Canvas>();
        canvas.renderMode = RenderMode.ScreenSpaceOverlay;

        UnityEngine.UI.CanvasScaler scaler = obj.AddComponent<UnityEngine.UI.CanvasScaler>();
        scaler.uiScaleMode =  UnityEngine.UI.CanvasScaler.ScaleMode.ScaleWithScreenSize;

        obj.AddComponent<UnityEngine.UI.GraphicRaycaster>();
        return obj; 
    }


    public 
    GameObject PanelCreate(
                            GameObject obj, 
                            string name, 
                            Vector2 position,
                            Vector2 size
                        )
    { 
        GameObject panelObject = new GameObject(name);
        panelObject.transform.SetParent(obj.transform);

        RectTransform rect = RectTransformCreate(panelObject, position, size); 

        // UnityEngine.UI.RawImage ?? 
        UnityEngine.UI.Image panelImage = panelObject.AddComponent<UnityEngine.UI.Image>();
        panelImage.color = new Color(1, 1, 1, 0.5f); 

        return panelObject;
    }


    public // static
    RectTransform RectTransformCreate(GameObject obj, Vector2 position, Vector2 size) 
    {
        float xsize = size.x; //0.2f; 
        float ysize = size.y; //0.3f; 

        float xmin = position.x; // 0.1f; // 0:|- ; 1:-|
        float ymin = position.y; // 0.2f; // 0:bottom; 1:top

        xmin = 1f - xmin; 
        ymin = 1f - ymin; 
        Vector2 min = new Vector2(1f, 1f) - new Vector2(xmin, ymin);
        Vector2 max = min + new Vector2(xsize, ysize); 

        RectTransform rect = obj.GetComponent<RectTransform>();
        if(rect == null) rect = obj.AddComponent<RectTransform>();

        rect.offsetMin = Vector2.zero;
        rect.offsetMax = Vector2.zero;
        rect.anchorMin = min; // new Vector2(0, 0);
        rect.anchorMax = max; // new Vector2(1, 1);
        return rect; 
    }

} // CameraTools
*/

} // SpicyTech