using System;
using System.Collections.Generic;

using UnityEngine;
using UnityEngine.UIElements;
using UnityEngine.EventSystems;


///////////////////////////////////////////////////////////////////////////////





///////////////////////////////////////////////////////////////////////////////
public 
class InteractionMouseOver : MonoBehaviour, IPointerEnterHandler, IPointerExitHandler
{
    public Camera cam; 


    public 
    void Start()
    {
        obj = this.gameObject; 
        parent = obj.transform.parent.gameObject; 

        Debug.Log($"[InteractionMouseOver] Awake:'{obj.name}' ({parent.name}, {cam.name}) ");
    }


    public 
    void OnPointerEnter(PointerEventData eventData)
    {
        Debug.Log($"[InteractionMouseOver] Entering: '{obj.name}' ({parent.name}) ");
    }


    public 
    void OnPointerExit(PointerEventData eventData)
    {
        Debug.Log($"[InteractionMouseOver] Exiting: '{obj.name}' ");
    }


    public 
    void Awake()
    {
    }


    GameObject obj = null; 
    GameObject parent = null; 
}



///////////////////////////////////////////////////////////////////////////////
public 
struct CameraControl
{
    Camera camera; 

    public float rotationSpeed; 
    public float fieldOfView; 
    public float radius; 

    Vector3 angles; 
    Vector3 center; 
    Vector3 dragPosition; 
    Vector3 startPosition; 

    public
    void Init(GameObject obj, Camera camera) 
    {
        GeometryUtils.EventTriggerPointerClickadd(obj.transform, camera); 

        this.camera = camera; 
        fieldOfView = camera.fieldOfView; 

        radius = 1.0f; 
        angles = new Vector3(); 
        rotationSpeed = Time.deltaTime;

        center = new Vector3(0.0f, 0.4f, 0.0f); 
    }


    public
    void Move(int direction) 
    {
        radius += 1.0f * direction * rotationSpeed; 
        //fieldOfView += 10.0f * rotationSpeed * direction * ((radius < 0.01f)?(0.0f):(1.0f)) ;

        radius = Mathf.Clamp(radius, 0.01f, 1.5f);
        fieldOfView = Mathf.Clamp(fieldOfView, 30f, 100f); 

        camera.fieldOfView = fieldOfView; 

        angles.y = Mathf.Clamp(angles.y, -85f, 85f);
        angles.x = Mathf.Clamp(angles.x, -185f, 185f);

        GeometryUtils.PositionCameraAroundObject(camera, 
            center,  
            angles[0], 
            -angles[1], 
            radius
        ); 
    }


    public 
    bool CenterClick(Vector3 center) 
    {
        this.center = center; 
        return true; 
    } 


    public 
    bool LeftClick() 
    {
        return true; 
    } 


    public 
    bool RightClick() 
    {
        Vector3 dragDelta = (Vector3)(Input.mousePosition - dragPosition) * rotationSpeed;
        dragDelta.x = Mathf.Clamp(dragDelta.x, -1f, 1f);
        dragDelta.y = Mathf.Clamp(dragDelta.y, -1f, 1f);
        dragDelta.z = Mathf.Clamp(dragDelta.z, -1f, 1f);
        angles += dragDelta * 0.25f; 
        return true; 
    }


    public 
    void RightUp() 
    {
        dragPosition = Input.mousePosition;
        startPosition = camera.transform.position;
    }

}


///////////////////////////////////////////////////////////////////////////////
public 
struct Selected 
{
    Material[] originals; 
    int layer; 

    public GameObject obj; 

    public
    void Reset()
    {
        Renderer renderer = obj.GetComponent<Renderer>();
        renderer.materials = originals; 

        obj.layer = layer; 

        obj = null; 
    }    


    public 
    void Touched(GameObject obj, GameObject bbox, int layer)
    {
        this.obj = obj; 

        Renderer renderer = obj.GetComponent<Renderer>();
        if (renderer != null)
        {
            originals = renderer.materials;
            Material highlightMaterial = new Material(renderer.material);
            highlightMaterial.color = Color.yellow;
            renderer.material = highlightMaterial;
        }

        this.layer = obj.layer; 
        obj.layer = layer; 
    }  
}


///////////////////////////////////////////////////////////////////////////////
public 
struct Chosen  
{
    Material[] originals; 

    GameObject obj; 
    GameObject copy; 


    public
    void Reset()
    {
        obj = null; 
        copy = null; 
    }    


    public 
    GameObject ObjectGet()
    {
        Vector3 local = obj.transform.position; 
        //Vector3 global = obj.transform.TransformPoint(local);
        Vector3 global = obj.GetComponent<Collider>().bounds.center; 

        Debug.Log($" ObjectGet:'{ obj.name }' '{local}' '{global}' "); 
        return obj; 
    }


    public 
    Vector3 CenterGet()
    {
        Vector3 global = obj.GetComponent<Collider>().bounds.center; 
        return global; 
    }


    public 
    bool CenterClick(GameObject obj, GameObject bbox, int layer) 
    {
        if(obj == null) return false; 
        originals = obj.GetComponent<Renderer>().materials;

        obj.GetComponent<Renderer>().material.color = Color.blue; 
//        GeometryUtils.AddBoundingBoxEdges(obj, bbox, Color.blue, 0.004f, layer); 

        copy = null; 
        this.obj = obj; 
        return true; 
    } 


    public
    void DoSomething()
    {
        if (obj == null) return; 

        if(copy == null) 
        {
            obj.GetComponent<Renderer>().materials = originals; 
        }
    }    

} 


///////////////////////////////////////////////////////////////////////////////
public 
struct CamerasController  
{
    public List<Camera> cameras;

    public 
    void Clear()
    {
        GeometryUtils.ClearCameraList(ref cameras);

        Debug.Log($"[CamerasController] 'OnDisable' ");
    }


    public 
    void Start()
    {
        cameras = null; 
    }


    public 
    void Update()
    {
        if(cameras != null) return; 

        Vector3 rot;
        Vector3 pos;

        cameras = new List<Camera>();
        rot = new Vector3(0.0f, 180.0f, 0.0f);

        pos = new Vector3(0.0f, 0.5f, 0.5f);
        cameras.Add(GeometryUtils.CreateCamera(pos, rot, "cameraA"));

        pos = new Vector3(0.0f, 0.5f, 1.0f);
        cameras.Add(GeometryUtils.CreateCamera(pos, rot, "cameraB"));

        pos = new Vector3(0.0f, 0.5f, 1.5f);
        cameras.Add(GeometryUtils.CreateCamera(pos, rot, "cameraC"));

        foreach (Camera c in cameras) c.cullingMask = -1;
    }

}


public static class GeometryUtils
{

    public 
    struct CameraMovement 
    {
        public Camera camera;
        public Vector3 targetPosition;  
        public float horizontalAngle; 
        public float verticalAngle; 
        public float radius;      
    }


    public static 
    void NormalizeGeometry(GameObject rootObject)
    {
        if (rootObject == null)
        {
            Debug.LogError("Root object is null.");
            return;
        }

        // Get all MeshFilters in the hierarchy
        MeshFilter[] meshFilters = rootObject.GetComponentsInChildren<MeshFilter>();

        if (meshFilters.Length == 0)
        {
            Debug.LogError("No MeshFilters found in the object hierarchy.");
            return;
        }

        // Calculate the combined bounds of all meshes
        Bounds combinedBounds = new Bounds(rootObject.transform.position, Vector3.zero);
        foreach (MeshFilter meshFilter in meshFilters)
        {
            if (meshFilter.sharedMesh != null)
            {
                // Transform the local bounds to world bounds
                Bounds worldBounds = meshFilter.sharedMesh.bounds;
                worldBounds.center = meshFilter.transform.TransformPoint(worldBounds.center);
                combinedBounds.Encapsulate(worldBounds);
            }
        }

        // Get the size of the combined bounds
        Vector3 size = combinedBounds.size;

        float maxDimension = Mathf.Max(size.x, size.y, size.z);
        if (maxDimension == 0)
        {
            Debug.LogWarning("Combined geometry has zero dimensions, cannot normalize.");
            return;
        }

        // Calculate the scale factor and normalize the root object's scale
        float scaleFactor = 1.0f / maxDimension;
        rootObject.transform.localScale *= scaleFactor;

        Debug.Log($"[NormalizeGeometry] '{rootObject.name}' with scale factor: '{scaleFactor}' ");
        LogObjectBounds(rootObject); 
        //AddBoundingBoxEdges(rootObject, new GameObject("bbox1"), Color.green, 0.001f, "Default"); 
    }


    public static void LogObjectBounds(GameObject rootObject)
    {
        if (rootObject == null)
        {
            Debug.LogError("Root object is null.");
            return;
        }

        MeshFilter[] meshFilters = rootObject.GetComponentsInChildren<MeshFilter>();

        if (meshFilters.Length == 0)
        {
            Debug.LogError("No MeshFilters found in the object hierarchy.");
            return;
        }

        // Calculate combined bounds
        Bounds combinedBounds = new Bounds(rootObject.transform.position, Vector3.zero);
        foreach (MeshFilter meshFilter in meshFilters)
        {
            if (meshFilter.sharedMesh != null)
            {
                Bounds worldBounds = meshFilter.sharedMesh.bounds;
                worldBounds.center = meshFilter.transform.TransformPoint(worldBounds.center);
                combinedBounds.Encapsulate(worldBounds);
            }
        }

        // Log bounds size
        Vector3 size = combinedBounds.size;
        Debug.Log($"Combined Bounds of {rootObject.name}: Size = {size}, Largest Dimension = {Mathf.Max(size.x, size.y, size.z)}");
    }


    public static void AddBoundingBox(GameObject targetObject, Color boxColor)
    {
        if (targetObject == null)
        {
            Debug.LogError("Target object is null.");
            return;
        }

        // Calculate combined bounds of the object
        Bounds combinedBounds = new Bounds(targetObject.transform.position, Vector3.zero);
        MeshRenderer[] renderers = targetObject.GetComponentsInChildren<MeshRenderer>();

        if (renderers.Length == 0)
        {
            Debug.LogError("No renderers found on the object.");
            return;
        }

        foreach (var renderer in renderers)
        {
            combinedBounds.Encapsulate(renderer.bounds);
        }

        // Create a cube to represent the bounding box
        GameObject bbox = GameObject.CreatePrimitive(PrimitiveType.Cube);
        bbox.name = $"{targetObject.name}_BoundingBox";

        // Disable the collider of the cube
        Collider bboxCollider = bbox.GetComponent<Collider>();
        if (bboxCollider != null) UnityEngine.Object.Destroy(bboxCollider);

        // Match the position, size, and scale of the bounding box to the combined bounds
        bbox.transform.position = combinedBounds.center;
        bbox.transform.localScale = combinedBounds.size;

        // Set the material to make the bounding box visible
        Renderer bboxRenderer = bbox.GetComponent<Renderer>();
        bboxRenderer.material = new Material(Shader.Find("Unlit/Color"))
        {
            color = boxColor
        };

        // Make the bounding box a child of the target object
        bbox.transform.SetParent(targetObject.transform, true);
    }

public static 
void AddBoundingBoxEdges(
        GameObject targetObject, 
        ref GameObject bboxEdges, 
        Color edgeColor, 
        float thickness, 
        int layer)
{
    if (targetObject == null)
    {
        Debug.LogError("Target object is null.");
        return;
    }

    Bounds combinedBounds = new Bounds(targetObject.transform.position, Vector3.zero);
    MeshRenderer[] renderers = targetObject.GetComponentsInChildren<MeshRenderer>();

    if (renderers.Length == 0)
    {
        Debug.Log($"No Children found on the object '{targetObject.name}'");
        return;
    }

    foreach (var renderer in renderers)
    {
        combinedBounds.Encapsulate(renderer.bounds);
    }

    if (bboxEdges == null)
    {
        bboxEdges = new GameObject($"BBox_{targetObject.name}");
        bboxEdges.transform.SetParent(targetObject.transform, true);

        for (int i = 0; i < 12; i++)
        {
            GameObject edge = new GameObject($"Edge_{i}");
            edge.transform.SetParent(bboxEdges.transform, false);
            edge.layer = layer;

            LineRenderer lineRenderer = edge.AddComponent<LineRenderer>();
            lineRenderer.startWidth = thickness;
            lineRenderer.endWidth = thickness;
            lineRenderer.material = new Material(Shader.Find("Unlit/Color")) { color = edgeColor };
            lineRenderer.positionCount = 2;
        }
    }

    Vector3[] corners = new Vector3[8];
    Vector3 min = combinedBounds.min;
    Vector3 max = combinedBounds.max;

    corners[0] = new Vector3(min.x, min.y, min.z);
    corners[1] = new Vector3(max.x, min.y, min.z);
    corners[2] = new Vector3(max.x, max.y, min.z);
    corners[3] = new Vector3(min.x, max.y, min.z);
    corners[4] = new Vector3(min.x, min.y, max.z);
    corners[5] = new Vector3(max.x, min.y, max.z);
    corners[6] = new Vector3(max.x, max.y, max.z);
    corners[7] = new Vector3(min.x, max.y, max.z);

    int[,] edges = new int[12, 2]
    {
        { 0, 1 }, { 1, 2 }, { 2, 3 }, { 3, 0 },
        { 4, 5 }, { 5, 6 }, { 6, 7 }, { 7, 4 },
        { 0, 4 }, { 1, 5 }, { 2, 6 }, { 3, 7 }
    };

    for (int i = 0; i < edges.GetLength(0); i++)
    {
        LineRenderer lineRenderer = bboxEdges.transform.Find($"Edge_{i}").GetComponent<LineRenderer>();
        lineRenderer.SetPosition(0, corners[edges[i, 0]]);
        lineRenderer.SetPosition(1, corners[edges[i, 1]]);
    }

    bboxEdges.layer = layer;
}


/*
    public static void AddBoundingBoxEdges(
            GameObject targetObject, 
            GameObject bboxEdges, 
            Color edgeColor, 
            float thickness, 
            int layer)
    {
        if (targetObject == null)
        {
            Debug.LogError("Target object is null.");
            return;
        }

        // Calculate combined bounds of the object
        Bounds combinedBounds = new Bounds(targetObject.transform.position, Vector3.zero);
        MeshRenderer[] renderers = targetObject.GetComponentsInChildren<MeshRenderer>();

        if (renderers.Length == 0)
        {
            Debug.Log($"No Children found on the object '{targetObject.name}'");
            return;
        }

        foreach (var renderer in renderers)
        {
            combinedBounds.Encapsulate(renderer.bounds);
        }

        // Create an empty GameObject to hold the line renderer components
        //GameObject bboxEdges = bbox; //new GameObject(bboxName);
        bboxEdges.transform.position = combinedBounds.center;
        //bboxEdges.transform.SetParent(targetObject.transform, true);

        // Define the 8 corners of the bounding box
        Vector3[] corners = new Vector3[8];
        Vector3 min = combinedBounds.min;
        Vector3 max = combinedBounds.max;

        corners[0] = new Vector3(min.x, min.y, min.z);
        corners[1] = new Vector3(max.x, min.y, min.z);
        corners[2] = new Vector3(max.x, max.y, min.z);
        corners[3] = new Vector3(min.x, max.y, min.z);
        corners[4] = new Vector3(min.x, min.y, max.z);
        corners[5] = new Vector3(max.x, min.y, max.z);
        corners[6] = new Vector3(max.x, max.y, max.z);
        corners[7] = new Vector3(min.x, max.y, max.z);

        // Define the 12 edges of the bounding box using pairs of corners
        int[,] edges = new int[12, 2]
        {
            { 0, 1 }, { 1, 2 }, { 2, 3 }, { 3, 0 }, // Bottom edges
            { 4, 5 }, { 5, 6 }, { 6, 7 }, { 7, 4 }, // Top edges
            { 0, 4 }, { 1, 5 }, { 2, 6 }, { 3, 7 }  // Vertical edges
        };

        // Create LineRenderers for each edge
        ClearChildren(bboxEdges); 
        for (int i = 0; i < edges.GetLength(0); i++)
        {
            string edgeName = $"Edge_{i}";
            Transform exist = bboxEdges.transform.Find(edgeName);

            if (exist != null)
            {
                GameObject.Destroy(exist.gameObject);
            }            

            GameObject edge = new GameObject(edgeName);
            edge.transform.SetParent(bboxEdges.transform, false);
            edge.layer = layer; //LayerMask.NameToLayer(layerName); 

            LineRenderer lineRenderer = edge.AddComponent<LineRenderer>();
            lineRenderer.startWidth = thickness; // Adjust thickness of edges
            lineRenderer.endWidth = thickness;
            lineRenderer.material = new Material(Shader.Find("Unlit/Color"))
            {
                color = edgeColor
            };
            lineRenderer.positionCount = 2;

            lineRenderer.SetPosition(0, corners[edges[i, 0]]);
            lineRenderer.SetPosition(1, corners[edges[i, 1]]);
        }

        //bboxEdges.transform.SetParent(targetObject.transform, true);
        bboxEdges.layer = layer; //LayerMask.NameToLayer(layerName); 
    }
*/

    public static 
    void ClearChildren(GameObject parent)
    {
        foreach (Transform child in parent.transform)
        {
            ClearChildren(child.gameObject);
            GameObject.Destroy(child.gameObject);
        }
    }


    public static 
    Dictionary<string, List<GameObject>> GetParentChildRelationships(GameObject parent)
    {
        Dictionary<string, List<GameObject>> relationship = new Dictionary<string, List<GameObject>>();

        void TraverseAndStoreRelationships(GameObject currentParent)
        {
            List<GameObject> childrenList = new List<GameObject>();

            foreach (Transform child in currentParent.transform)
            {
                childrenList.Add(child.gameObject);
                TraverseAndStoreRelationships(child.gameObject);
            }

            if (childrenList.Count > 0)
            {
                relationship[currentParent.name] = childrenList;
            }
        }

        TraverseAndStoreRelationships(parent);
        return relationship;
    }


    public static 
    Dictionary<string, List<GameObject>> GetChildParentRelationships(GameObject root)
    {
        Dictionary<string, List<GameObject>> relationship = new Dictionary<string, List<GameObject>>();

        void TraverseAndStoreRelationships(GameObject currentParent)
        {
            foreach (Transform child in currentParent.transform)
            {
                if (!relationship.ContainsKey(child.name))
                {
                    relationship[child.name] = new List<GameObject>();
                }

                relationship[child.name].Add(currentParent);

                TraverseAndStoreRelationships(child.gameObject);
            }
        }

        TraverseAndStoreRelationships(root);
        return relationship;
    }


    public static 
    void AddCloseButton(UIDocument uiDocument)
    {
        Button closeButton = new Button(() =>
        {
            uiDocument.rootVisualElement.style.display = DisplayStyle.None;
        })
        {
            text = "X"
        };

        closeButton.style.position = Position.Relative;
        closeButton.style.top = 10;
        closeButton.style.right = -5;
        closeButton.style.width = 30;
        closeButton.style.height = 30;
        closeButton.style.backgroundColor = new StyleColor(Color.red);
        closeButton.style.color = new StyleColor(Color.white);

        var root = uiDocument.rootVisualElement;
        var cameraViewElement = root.Q<VisualElement>("cameraViewElement");
        cameraViewElement.Add(closeButton);
    }



    public static 
    void AddSlider(UIDocument uiDocument, string name, 
        SliderDirection direction, 
        int width, 
        int height, 
        int left, 
        int bottom)
    {
        Slider slider = new Slider
        {
            direction = direction,
            style =
            {
                position = Position.Relative, // position = Position.Absolute,
                bottom = new Length(bottom, LengthUnit.Percent), 
                width = new Length(width, LengthUnit.Percent),
                height = new Length(height, LengthUnit.Percent),
                left = new Length(left, LengthUnit.Percent),
                backgroundColor = new StyleColor(Color.gray),
            }
        };
        slider.name = name; 

        var root = uiDocument.rootVisualElement;
        var parentElement = root.Q<VisualElement>("cameraViewElement");
        parentElement.Add(slider);
    }


    public static 
    void ConfigureSlider(UIDocument uiDocument, 
            string name, 
            float min, 
            float max, 
            Action<float> functor
        )
    {
        var root = uiDocument.rootVisualElement;
        Slider slider = root.Q<Slider>(name); 

        var thumb = slider.Q("unity-slider-thumb");

        //slider.name = name; 
        slider.value = (min + max) * 0.125f;
        slider.lowValue = min;
        slider.highValue = max;

        slider.RegisterValueChangedCallback(evt => {
            functor(evt.newValue); 
            Debug.Log($"Slider value: {evt.newValue}");
        });
    }



    public static 
    void CameraRotation(
                            ref CameraMovement cm, 
                            Vector3? targetPosition = null, 
                            float? horizontalAngle = null, 
                            float? verticalAngle = null,                                                         
                            float? radius = null 
                        ) 
    {
        if (targetPosition != null) cm.targetPosition = targetPosition.Value;
        if (horizontalAngle != null) cm.horizontalAngle = horizontalAngle.Value;
        if (verticalAngle != null) cm.verticalAngle = verticalAngle.Value;
        if (radius != null) cm.radius = radius.Value;

        PositionCameraAroundObject(cm.camera, 
                                    cm.targetPosition, 
                                    cm.horizontalAngle, 
                                    cm.verticalAngle, 
                                    cm.radius
                                );

    }


    public static
    void PositionCameraAroundObject(Camera camera, 
        Vector3 targetPosition, 
        float horizontalAngle, 
        float verticalAngle, 
        float radius)
    {
        float verticalRad = Mathf.Deg2Rad * verticalAngle;
        float horizontalRad = Mathf.Deg2Rad * horizontalAngle;

        float x = targetPosition.x + radius * Mathf.Cos(verticalRad) * Mathf.Sin(horizontalRad);
        float y = targetPosition.y + radius * Mathf.Sin(verticalRad);
        float z = targetPosition.z + radius * Mathf.Cos(verticalRad) * Mathf.Cos(horizontalRad);

        camera.transform.position = new Vector3(x, y, z);
        camera.transform.LookAt(targetPosition);
    }


    public static
    Camera CreateCamera(Vector3 position, Vector3 rotation, string name, string layer="Default")
    {
        GameObject cameraObject = new GameObject(name);

        Camera camera = cameraObject.AddComponent<Camera>();
        camera.transform.position = position;
        camera.transform.rotation = Quaternion.Euler(rotation);

        camera.clearFlags = CameraClearFlags.SolidColor;
        camera.backgroundColor = new Color(0.1f, 0.1f, 0.1f); 
        camera.nearClipPlane = 0.01f; 

        camera.cullingMask &= -1; //(1 << LayerMask.NameToLayer(layer));

        CamaraPhysicsRaycasterAdd(camera); 
        return camera; 
    }


    public static 
    Vector2 GetNormalizedMousePosition(Camera camera)
    {
        Vector3 mousePosition = Input.mousePosition;
        float normalizedX = mousePosition.x / Screen.width;
        float normalizedY = mousePosition.y / Screen.height;

        float mappedX = (normalizedX * 2f) - 1f;
        float mappedY = (normalizedY * 2f) - 1f;

        return new Vector2(mappedX, mappedY);
    }


    public static
    void GetPointerNormalizedPosition(UIDocument uiDocument, Camera targetCamera)
    {
        var root = uiDocument.rootVisualElement;

        root.RegisterCallback<PointerMoveEvent>(evt =>
        {
            Vector2 pointerScreenPos = evt.position;
            Vector2 normalizedPos = ScreenToNormalized(pointerScreenPos, targetCamera);
            Debug.Log($"Pointer Normalized Position: {normalizedPos}");
        });
    }

    private static
    Vector2 ScreenToNormalized(Vector2 screenPosition, Camera camera)
    {
        Vector3 worldPos = camera.ScreenToWorldPoint(screenPosition);
        Vector3 viewportPos = camera.WorldToViewportPoint(worldPos);
        return new Vector2(viewportPos.x * 2 - 1, viewportPos.y * 2 - 1);
    }


    public static
    void EventTriggerPointerClickadd(Transform parent, Camera camera)
    {
        CamaraPhysicsRaycasterAdd(camera); 

        GameObject obj = parent.gameObject;
        void functor(PointerEventData data) 
        {
            Debug.Log($"[EventTriggerPointerClickadd] '{obj.name}' ({data.position})"); 
        }

        ObjectPhysicsRaycasterAdd(obj, functor); 
        foreach(Transform c in parent) EventTriggerPointerClickadd(c, camera);
    }


    public static
    void CameraPhysicsRaycastAdd(
                                    Camera camera, 
                                    float x, 
                                    float y, 
                                    ref GameObject touched, 
                                    int layerToIgnore = -1
                                )
    {
        LayerMask mask = ~(1 << layerToIgnore);

        Ray ray = camera.ScreenPointToRay(new Vector3(x, y, 0));

        if (Physics.Raycast(ray, out RaycastHit hit, Mathf.Infinity, mask))
        {
            touched = hit.collider.gameObject; 
            //Debug.Log($"[ObjectPhysicsRaycastAdd] Hit: '{touched.name}' ");
        }
    }


    public static
    void CamaraPhysicsRaycasterAdd(Camera camera)
    {
        PhysicsRaycaster rc = camera.gameObject.GetComponent<PhysicsRaycaster>();
        if(rc == null)  
        {
            rc = camera.gameObject.AddComponent<PhysicsRaycaster>();
            Debug.Log($"[CamaraPhysicsRaycasterAdd] 'PhysicsRaycaster' added to camera:'{camera.name}' "); 
        }
    }


    public static
    void ObjectPhysicsRaycasterAdd(
                                    GameObject obj, 
                                    Action<PointerEventData> functor = null
                                  )
    {
        if(obj == null) 
        {
            Debug.LogWarning($"[ObjectPhysicsRaycasterAdd] no 'GameObject' !!");
            return;
        }

        MeshFilter mf = obj.GetComponent<MeshFilter>(); 
        if (mf == null) 
        {
            Debug.LogWarning($"[ObjectPhysicsRaycasterAdd] no 'MeshFilter' ({obj.name}) !!");
            return;
        }

        Rigidbody rb = obj.GetComponent<Rigidbody>();
        if (rb == null) 
        {
            Debug.LogWarning($"[ObjectPhysicsRaycasterAdd] no 'Collider' ({obj.name}) !!");
            return;
        }


        if(functor == null) functor = Listener; 

        EventTrigger eventTrigger = obj.GetComponent<EventTrigger>();
        if (eventTrigger == null) eventTrigger = obj.AddComponent<EventTrigger>();

        EventTrigger.Entry existingEntry = eventTrigger.triggers.Find(entry => entry.eventID == EventTriggerType.PointerClick);
        if (existingEntry == null) 
        {
            EventTrigger.Entry entry = new EventTrigger.Entry();
            entry.eventID = EventTriggerType.PointerClick;
            entry.callback = new EventTrigger.TriggerEvent();
            entry.callback.AddListener((data) => functor((PointerEventData)data));
            eventTrigger.triggers.Add(entry);
        }
    }


    public static
    void Listener(PointerEventData eventData)
    {
        Debug.Log($"[ObjectPhysicsRaycasterAdd] Pointer Position: '{eventData.position}' ");
    }


    public static
    void SetMainCamera(Camera newMainCamera)
    {
        Camera currentMainCamera = Camera.main;
        if (currentMainCamera != null) currentMainCamera.tag = "Untagged"; 

        newMainCamera.tag = "MainCamera";
        Debug.Log($"[SetMainCamera] New main camera set: {newMainCamera.name}");
    }


    public static
    void ClearUnityObjectList<T>(ref List<T> list) where T : UnityEngine.Object
    {
        if (list == null || list.Count == 0)
        {
            Debug.Log("[ClearUnityObjectList] The list is already null or empty.");
            return;
        }

        foreach (T item in list)
        {
            if (item != null)
            {
                if (item is GameObject go)
                {
                    UnityEngine.Object.Destroy(go);
                }
                else
                    UnityEngine.Object.Destroy(item);
            }
        }

        list.Clear(); // Clear the list
        list = null;  // Set the list to null
        Debug.Log("[ClearUnityObjectList] All Unity objects in the list have been destroyed, and the list is set to null.");
    }


    public static 
    void ClearCameraList(ref List<Camera> cameras)
    {
        if (cameras == null || cameras.Count == 0)
        {
            Debug.Log("[ClearCameraList] The list is already null or empty.");
            return;
        }

        foreach (Camera camera in cameras)
        {
            if (camera != null)
            {
                PhysicsRaycaster raycaster = camera.GetComponent<PhysicsRaycaster>();
                if (raycaster != null) UnityEngine.Object.Destroy(raycaster);
                UnityEngine.Object.Destroy(camera.gameObject);
            }
        }

        cameras.Clear();
        cameras = null;
        Debug.Log("[ClearCameraList] done!!");
    }


    public static
    GameObject CreatePrimitivePlaneXZ()
    {
        GameObject plane = GameObject.CreatePrimitive(PrimitiveType.Plane);
        //plane.transform.localScale = new Vector3(0.5f, 1f, 0.4f); 

        Rigidbody rigidbody = plane.AddComponent<Rigidbody>();
        rigidbody.isKinematic = true;

        Material material = new Material(Shader.Find("Standard"));

        material.renderQueue = 2450; 
        material.SetFloat("_Metallic", 0f); // Set metallic to 0
        material.SetInt("_SrcBlend", (int)UnityEngine.Rendering.BlendMode.One);
        material.SetInt("_DstBlend", (int)UnityEngine.Rendering.BlendMode.Zero);
        material.SetInt("_ZWrite", 1);
        material.EnableKeyword("_ALPHATEST_ON");
        material.DisableKeyword("_ALPHABLEND_ON");
        material.DisableKeyword("_ALPHAPREMULTIPLY_ON");

        material.SetFloat("_Mode", 1f); // Cutout mode corresponds to value 1
        material.color = new Color(0f, 0f, 0f, 0f);
        material.SetFloat("_Cutoff", 1f); 

        Renderer renderer = plane.GetComponent<Renderer>();
        renderer.material = material;

        return plane; 
    }



    public static 
    void CreateRedCube(Vector3 position, Vector3 scale)
    {
        GameObject cube = GameObject.CreatePrimitive(PrimitiveType.Cube);
        cube.transform.localScale = scale; //new Vector3(0.1f, 0.2f, 0.3f);
        cube.transform.position = position; //new Vector3(0f, 3f, 0f);

        Material material = new Material(Shader.Find("Standard"));
        material.color = Color.red; 

        Renderer renderer = cube.GetComponent<Renderer>();
        renderer.material = material;

        Rigidbody rigidbody = cube.AddComponent<Rigidbody>();
        rigidbody.isKinematic = false;
    }


    public static 
    void AddBoxColliderAndRigidbody(GameObject obj)
    {
        if (obj == null) return;

        if (!obj.GetComponent<BoxCollider>())
        {
            BoxCollider collider = obj.AddComponent<BoxCollider>();
            //BoxCollider.convex = true; 
        }

        if (!obj.GetComponent<Rigidbody>())
        {
            obj.AddComponent<Rigidbody>();
        }
    }


    public static void ColliderAdd<T>(GameObject obj, bool convex) where T : Collider
    {
        MeshFilter meshFilter = obj.GetComponent<MeshFilter>(); 
        if(meshFilter == null) return; 

        Collider existingCollider = obj.GetComponent<Collider>();
        if (existingCollider == null)
        {
            T newCollider = obj.AddComponent<T>();
            
            if (newCollider is MeshCollider meshCollider)
            {
                meshCollider.convex = convex;
            }

            PhysicsMaterial material = new PhysicsMaterial();
            material.bounciness = 1.0f;
            material.staticFriction = 0.075f;
            material.dynamicFriction = 0.075f;
            material.bounceCombine = PhysicsMaterialCombine.Minimum;  
            material.frictionCombine = PhysicsMaterialCombine.Minimum;  
            newCollider.material = material;
        }
    }


    public static
    void MeshColliderAdd(Transform parent, bool convex)
    {
        ColliderAdd<MeshCollider>(parent.gameObject, convex);
        foreach(Transform child in parent) MeshColliderAdd(child, convex);
    }


    public static
    void BoxColliderAdd(Transform parent)
    {
        ColliderAdd<BoxCollider>(parent.gameObject, false);
        foreach(Transform child in parent) BoxColliderAdd(child);
    }

    public static
    void LayerAdd(Transform parent, int layer)
    {
        parent.gameObject.layer = layer;
        foreach(Transform child in parent) LayerAdd(child, layer);
    }


    public static
    void RigidbodyAdd(Transform parent)
    {
        GameObject obj = parent.gameObject; 

        MeshFilter mf = obj.GetComponent<MeshFilter>(); 

        Rigidbody rb = obj.GetComponent<Rigidbody>();
        if((rb == null) && (mf != null) ) 
        {
            rb = obj.AddComponent<Rigidbody>();
            //rb.mass = 1f;
            //rb.interpolation = RigidbodyInterpolation.Interpolate;
            //rb.collisionDetectionMode = CollisionDetectionMode.Continuous;
            //rb.solverIterations = 100;
        }

        foreach(Transform child in parent) RigidbodyAdd(child);
    }


    public static 
    void RaycastOnMesh(GameObject obj, Camera camera, float x, float y, ref Vector3 hitPoint)
    {
        MeshCollider collider = obj.GetComponent<MeshCollider>(); 
        if(collider == null) Debug.LogError($"hit.point:'{hitPoint}'");  

        Ray ray = camera.ScreenPointToRay( new Vector3(x, y, 0) );
        if (collider.Raycast(ray, out RaycastHit hit, Mathf.Infinity))
        {
            hitPoint = hit.point;
        }
    }


    public static
    GameObject CreatePrimitiveSphere()
    {
        GameObject obj = GameObject.CreatePrimitive(PrimitiveType.Sphere);
        //sphere.transform.position = point;
        //sphere.transform.localScale = new Vector3(0.01f, 0.01f, 0.01f);

        //Rigidbody rigidbody = obj.AddComponent<Rigidbody>();
        //rigidbody.isKinematic = true;

        Renderer renderer = obj.GetComponent<Renderer>();
        renderer.material = new Material(Shader.Find("Standard"));
        renderer.material.color = Color.red;
        return obj; 
    }


    public static 
    GameObject ShallowCopy(GameObject original)
    {
        GameObject copy = new GameObject(original.name + "_Copy");
        foreach (Component component in original.GetComponents<Component>())
        {
            if (component is Transform)
            {
                // Copy transform properties manually
                Transform originalTransform = (Transform)component;
                copy.transform.position = originalTransform.position;
                copy.transform.rotation = originalTransform.rotation;
                copy.transform.localScale = originalTransform.localScale;
            }
            else
            {
                // Copy other components
                System.Type type = component.GetType();
                Component copiedComponent = copy.AddComponent(type);
                foreach (var field in type.GetFields())
                {
                    field.SetValue(copiedComponent, field.GetValue(component));
                }
            }
        }

        return copy;
    }


    public static 
    void ComponentActivation(MonoBehaviour scriptComponent, bool isActive)
    {
        if (scriptComponent != null) scriptComponent.enabled = isActive;
    }


    public static 
    void PhysicsSimulationMode(SimulationMode mode)
    {
        Physics.simulationMode = mode; // Script | FixedUpdate | Update 
    }


    public static
    GameObject CanvasObjectCreate(string name)
    {
        GameObject obj = new GameObject(name);

        Canvas canvas = obj.AddComponent<Canvas>();
        canvas.renderMode = RenderMode.ScreenSpaceOverlay;

        UnityEngine.UI.CanvasScaler scaler = obj.AddComponent<UnityEngine.UI.CanvasScaler>();
        scaler.uiScaleMode =  UnityEngine.UI.CanvasScaler.ScaleMode.ScaleWithScreenSize;

        obj.AddComponent<UnityEngine.UI.GraphicRaycaster>();

        //CanvasGroup canvasGroup = obj.AddComponent<CanvasGroup>();
        //canvasGroup.blocksRaycasts = true;

        return obj; 
    }

    
    public static
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


    public static
    GameObject BackGroundImageCreate(GameObject obj, string name, Vector2 pos, Vector2 size, Color color) 
    {
        GameObject backgroundObject = new GameObject(name);
        backgroundObject.transform.SetParent(obj.transform);

        RectTransformCreate(backgroundObject, pos, size); 

        UnityEngine.UI.RawImage backgroundImage = backgroundObject.AddComponent<UnityEngine.UI.RawImage>();
        ////UnityEngine.UI.Image backgroundImage = backgroundObject.AddComponent<UnityEngine.UI.Image>();
        backgroundImage.color = color;    

        //CanvasGroup canvasGroup = backgroundObject.AddComponent<CanvasGroup>();
        //canvasGroup.blocksRaycasts = true;

        //backgroundImage.raycastTarget = true; // ?
        //backgroundObject.transform.SetAsFirstSibling(); // ?
        return backgroundObject; 
    }


    public static
    void Camera2TextureInit(Camera camera, 
        UnityEngine.UI.RawImage image, 
        ref RenderTexture renderTexture, 
        ref Texture2D texture2D, 
        int width, int height)
    {
        int BitsInDepth = 24;

        renderTexture = new RenderTexture(width, height, BitsInDepth);
        camera.targetTexture = renderTexture;

        texture2D = new Texture2D(width, height, TextureFormat.RGB24, false);
        image.texture = texture2D;
    }


    public static
    void Camera2TextureUpdate(RenderTexture renderTexture, Texture2D texture2D, int width, int height)
    {
        RenderTexture.active = renderTexture;
        texture2D.ReadPixels(new Rect(0, 0, width, height), 0, 0);
        texture2D.Apply();

        RenderTexture.active = null;
        //MarkerUpdate(cameraViewImage.rectTransform, visualMarker);
    }


    public static
    bool IsMouseOverRectTransform(RectTransform rectTransform)
    {
        Vector2 mousePosition = Input.mousePosition;
        Camera canvasCamera = null; // Use null for Screen Space - Overlay canvases

        if (RectTransformUtility.RectangleContainsScreenPoint(rectTransform, mousePosition, canvasCamera))
        {
            return true;
        }

        return false;
    }


    public static
    void GeometryControl(
        GameObject obj, 
        ref GameObject bbox1, 
        ref GameObject bbox2,
        ref GameObject plane, 
        ref GameObject sphere, 
        int layerInactive, 
        int layerRigidBodies)
    {
        //touched = null; 

        bbox1 = new GameObject("bbox1"); 
        bbox1.layer = layerInactive; 

        bbox2 = new GameObject("bbox2"); 
        bbox2.layer = layerInactive; 

        plane = GeometryUtils.CreatePrimitivePlaneXZ(); 
        plane.name = "ground";
        plane.layer = layerInactive; 
        plane.transform.position = new Vector3(0f, 0.0f, 0f);
        plane.transform.localScale = new Vector3(10f, 10f, 10f); 

        sphere = GeometryUtils.CreatePrimitiveSphere(); 
        sphere.name = "pointer";
        sphere.layer = layerInactive; 
        sphere.transform.localScale = new Vector3(0.025f, 0.025f, 0.025f); 

        GeometryUtils.NormalizeGeometry( obj ); // :) 
        GeometryUtils.BoxColliderAdd( obj.transform ); // :) 
        GeometryUtils.RigidbodyAdd( obj.transform ); // :) 
        GeometryUtils.LayerAdd( obj.transform, layerRigidBodies);
    }



    // public static
} // GeometryUtils