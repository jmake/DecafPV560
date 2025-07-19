using System;
using System.Collections.Generic;

using UnityEngine;
using UnityEngine.UI;
using UnityEngine.UIElements;
using UnityEngine.EventSystems;
using UnityEngine.SceneManagement;


///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
public 
struct CanvasController
{
    string canvasName; 
    public GameObject mainCanvasObject; 

    public 
    struct ButtonController
    {
        public UnityEngine.Events.UnityAction func; 
        public string message;
        public Vector2 position; 
        public Vector2 size;
    }    

    public Dictionary<string, ButtonController> dict2; 

    public Dictionary<string, GameObject> levels; 

    public GameObject ObjectRawImage; 

    Camera camera; 

    public 
    void Clear()
    {
        if (mainCanvasObject != null) 
        {
            UnityEngine.Object.Destroy(mainCanvasObject);
            mainCanvasObject = null;
        }

        if (dict2 != null) dict2 = null; 
        if (levels != null) levels = null; 
    }


    public 
    void Start(string canvasName, Camera camera)
    {
        this.camera = camera; 
        this.canvasName = canvasName; 

        mainCanvasObject = null; 
        dict2 = new Dictionary<string, ButtonController>(); 
        levels = new Dictionary<string, GameObject>(); 
    }


    public 
    void ButtonActionAdd(
            string name, 
            UnityEngine.Events.UnityAction func, 
            Vector2 position, 
            Vector2 size
        )
    {
        if( dict2.ContainsKey(name) ) return; 

        ButtonController c = new ButtonController(); 
        c.size = size; 
        c.func = func; 
        c.message = name; 
        c.position = position; 
        dict2.Add(name, c); 
    }


    public 
    void Update(Vector2 position, Vector2 size)
    {
        if (mainCanvasObject != null) return;  

        mainCanvasObject = GeometryUtils.CanvasObjectCreate(canvasName); 

        levels["level1"] = UILevel1(mainCanvasObject, position, size); 
        levels["level2"] = UILevel2(mainCanvasObject); 
    }

    public 
    void UILevel1Update()
    {
/*
        RectTransform rect = levels["level1"].GetComponent<RectTransform>(); 

        bool overRect = GeometryUtils.IsMouseOverRectTransform( rect ); 
        if(overRect) 
        {
//            Debug.Log($"Mouse is over:'{rect.name}'");
        }
*/
    }


    GameObject UILevel1(GameObject obj, Vector2 position, Vector2 size) 
    {
        GameObject mainPanel = PanelCreate(
                                            obj, 
                                            "UILevel1",
                                            position, size
                                        ); 

        GameObject rawImage_2 = GeometryUtils.BackGroundImageCreate(mainPanel, 
            "rawImage_2", 
            new Vector2(0.05f, 0.05f), 
            new Vector2(0.9f, 0.9f), 
            new Color(1.0f, 1.0f, 1.0f, 1.0f)
        ); 
        ObjectRawImage = rawImage_2; 

        InteractionsAdd(mainPanel); 
        
        foreach(var bc in dict2.Values) ButtonPlayCreate(mainPanel, bc); 

        return mainPanel; 
    }


    GameObject UILevel2(GameObject obj) 
    {
        GameObject rawImage_1 = GeometryUtils.BackGroundImageCreate(obj, 
            "UILevel2", 
            new Vector2(0.05f,0.05f), 
            new Vector2(0.90f,0.4f), 
            new Color(1.0f, 0.0f, 0.0f, 0.5f)
        ); 

        return rawImage_1; 
    }


    GameObject PanelCreate(
                            GameObject obj, 
                            string name, 
                            Vector2 position,
                            Vector2 size
                        )
    { 
        GameObject panelObject = new GameObject(name);
        panelObject.transform.SetParent(obj.transform);

        RectTransform rect = GeometryUtils.RectTransformCreate(panelObject, position, size); 

        // UnityEngine.UI.RawImage ?? 
        UnityEngine.UI.Image panelImage = panelObject.AddComponent<UnityEngine.UI.Image>();
        panelImage.color = new Color(1, 1, 1, 0.5f); 

        return panelObject;
    }


    void ButtonPlayCreate(GameObject obj, ButtonController bc)
    {
        UnityEngine.UI.Button button = ButtonCloseCreate(obj, bc.message, bc.position, bc.size); 
        button.onClick.AddListener( bc.func );
    }


    void InteractionsAdd(GameObject obj) 
    {
        // Interaction 1
        InteractionDraggable draggable = obj.AddComponent<InteractionDraggable>();

        // Interaction 2
        float delta = 0.1f; 
        Vector2 size = new Vector2(delta, delta); 
        Vector2 position = new Vector2(1f-delta, 1f-delta); 

        UnityEngine.UI.Button button = ButtonCloseCreate(obj, "-", position, size); 
        button.onClick.AddListener( () => Debug.Log("(-) clicked...") );

        InteractionMinimizable minimize = obj.AddComponent<InteractionMinimizable>();
        minimize.toggleButton = button;

        // Interaction 3
        UIPhysicsBlocker blocker = obj.AddComponent<UIPhysicsBlocker>();

        // Interaction 4
        InteractionMouseOver mouseOver = obj.AddComponent<InteractionMouseOver>(); 
        mouseOver.cam = camera; 
    }


    private 
    UnityEngine.UI.Button ButtonCloseCreate(GameObject panelObject, string message, Vector2 position, Vector2 size)
    {
        GameObject buttonObject = ButtonCreate(message);
        buttonObject.transform.SetParent(panelObject.transform);

        GameObject textObject = TextCreate(message); 
        textObject.transform.SetParent(buttonObject.transform);

        GeometryUtils.RectTransformCreate(buttonObject, position, size);  
        GeometryUtils.RectTransformCreate(textObject, new Vector2(), new Vector2(1f,1f));  

        UnityEngine.UI.Button button = buttonObject.GetComponent<UnityEngine.UI.Button>();
        return button; 
    }


    GameObject ButtonCreate(string message)
    {
        GameObject obj = new GameObject($"button_{message}");

        UnityEngine.UI.Button button = obj.AddComponent<UnityEngine.UI.Button>();
        button.targetGraphic = obj.AddComponent<UnityEngine.UI.Image>();
        button.targetGraphic.color = Color.red; 
        return obj; 
    }


    GameObject TextCreate(string message)
    {
        GameObject obj = new GameObject($"Text_{message}");

        Text buttonText = obj.AddComponent<Text>();
        buttonText.font = Resources.GetBuiltinResource<Font>("LegacyRuntime.ttf");
        buttonText.color = Color.white;
        buttonText.alignment = TextAnchor.MiddleCenter;

        buttonText.fontSize = 6;
        buttonText.resizeTextMinSize = 6;  
        buttonText.resizeTextMaxSize = 18; 
        buttonText.resizeTextForBestFit = true;

        buttonText.text = message;
        return obj; 
    }


}


///////////////////////////////////////////////////////////////////////////////
public class UIPhysicsBlocker : MonoBehaviour, IPointerEnterHandler, IPointerExitHandler
{
    private CanvasGroup canvasGroup;
    private bool isBlocking = false;


    private void Awake()
    {
        GameObject obj = this.gameObject; 
        canvasGroup = obj.AddComponent<CanvasGroup>();
        canvasGroup.blocksRaycasts = true;
    }


    void Update()
    {
        bool overObject = EventSystem.current.IsPointerOverGameObject();
        //Debug.Log( $"overObject:'{overObject}' " ); 
    }


    public void OnPointerEnter(PointerEventData eventData)
    {
        isBlocking = true;
    }

    public void OnPointerExit(PointerEventData eventData)
    {
        isBlocking = false;
    }

    public bool IsBlockingRaycast()
    {
        return isBlocking;
    }
}



///////////////////////////////////////////////////////////////////////////////
public class InteractionMinimizable : MonoBehaviour
{
    public UnityEngine.UI.Button toggleButton;
    //public Vector2 minimizedSize = new Vector2(100, 50);

    RectTransform rect;
    //Vector2 originalSize;
    bool isMinimized = false;

    Vector2 anchorMin; 
    Vector2 anchorMax; 
    GameObject panelObject; 

    private void Awake()
    {
        panelObject = this.gameObject; 
        Debug.Log( panelObject.name ); 

        rect = panelObject.GetComponent<RectTransform>();

        //originalSize = rect.sizeDelta;
        anchorMin = rect.anchorMin; 
        anchorMax = rect.anchorMax; 
    }


    private void Start()
    {
        if (toggleButton != null) toggleButton.onClick.AddListener(ToggleMinimize);
    }


    private void ToggleMinimize()
    {
        isMinimized = !isMinimized;
        //rect.sizeDelta = isMinimized ? minimizedSize : originalSize;

        float width = 0.1f; 
        float height = 0.1f; 
        Vector2 size = new Vector2(width, height); 
        Vector2 position = new Vector2(0.5f, 0.5f); 
        GeometryUtils.RectTransformCreate(panelObject, position, size); 
    }


    public void RestoreSize()
    {
        isMinimized = false;
        rect.anchorMin = anchorMin; 
        rect.anchorMax = anchorMax; 
        //rect.sizeDelta = originalSize;
    }
}


///////////////////////////////////////////////////////////////////////////////
public 
class InteractionDraggable : MonoBehaviour, IPointerDownHandler, IBeginDragHandler, IDragHandler, IEndDragHandler
{
    private Vector2 offset;
    private RectTransform rectTransform;


    private void Awake()
    {
        rectTransform = GetComponent<RectTransform>();
    }


    public void OnPointerDown(PointerEventData eventData)
    {
        RectTransformUtility.ScreenPointToLocalPointInRectangle(
            rectTransform,
            eventData.position,
            eventData.pressEventCamera,
            out offset
        );
    }


    public void OnDrag(PointerEventData eventData)
    {
        Vector2 localPoint;
        if (RectTransformUtility.ScreenPointToLocalPointInRectangle(
                rectTransform.parent as RectTransform,
                eventData.position,
                eventData.pressEventCamera,
                out localPoint))
        {
            rectTransform.localPosition = localPoint - offset; 
        }
    }

    public void OnEndDrag(PointerEventData eventData){}
    public void OnBeginDrag(PointerEventData eventData) {}
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
public class CanvasWindowPopUp : MonoBehaviour
{
    public string canvasName = "canvasName"; 
    public Camera cameraToControl = null;

    CanvasController canvasController = new CanvasController(); 

    Texture2D texture2D = null; 
    RenderTexture renderTexture = null; 

    int width2; 
    int hight2; 
    int BitsInDepth = 24;

    public float xmin1 = 0.1f;
    public float ymin1 = 0.9f; 
    public float width1 = 0.75f; 
    public float hight1 = 0.50f; 


    void OnDisable()
    {
        ButtonClose(); 
    }


    void Start()
    {
        GeometryUtils.PhysicsSimulationMode( SimulationMode.Script ); 

        canvasController.Start(canvasName, cameraToControl);

        Camera2TextureStart(); 
    }


    void Update()
    {
//        camerasController.Update();
        //camerasController.cameras["cameraA"]; 

        float delta = 0.1f; 
        canvasController.ButtonActionAdd(">", ButtonPlay, new Vector2(0f,-delta), new Vector2(delta,delta)); 
        canvasController.ButtonActionAdd("x", ButtonClose, new Vector2(0f,1f-delta), new Vector2(delta,delta)); 

        Vector2 size = new Vector2(width1, hight1); 
        Vector2 position = new Vector2(xmin1, ymin1); //ymin1-hight1);
        canvasController.Update(position, size);

        canvasController.UILevel1Update(); 

        Camera2TextureUpdate(); 
    }


    void Camera2TextureStart() 
    {
        if (texture2D != null)  UnityEngine.GameObject.Destroy(texture2D);
        if (renderTexture != null)  UnityEngine.GameObject.Destroy(renderTexture);

        width2 = (int)(Screen.width);
        hight2 = (int)(Screen.height);

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

        oir = canvasController.ObjectRawImage;
        image = oir.GetComponent<UnityEngine.UI.RawImage>();  
        image.texture = texture2D;

        RenderTexture.active = renderTexture;
        texture2D.ReadPixels(new Rect(0, 0, width2, hight2), 0, 0);
        texture2D.Apply();

        RenderTexture.active = null;
    }


    void ButtonClose() 
    {
//        cameraToControl.targetTexture = null; 
        renderTexture = null; 
        texture2D = null; 

        canvasController.Clear();
//        camerasController.Clear(); 

        Start(); 
        this.enabled = false;
    }


    void ButtonPlay()
    {
        GeometryUtils.PhysicsSimulationMode( SimulationMode.Update ); 
    }
}
///////////////////////////////////////////////////////////////////////////////