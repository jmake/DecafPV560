using System;
using System.Collections.Generic;

using UnityEngine;
using UnityEngine.EventSystems;


///////////////////////////////////////////////////////////////////////////////
public 
class ObjectMain : MonoBehaviour
{
    public GameObject obj;

    public int layerTouched; 
    public int layerInactive; 
    public int layerRigidBodies; 


    public
    void Update()
    {
        //if(bboxEdges != null) UnityEngine.Object.Destroy(bboxEdges); 
        //if(bbox2 != null) bbox2 = null; 

        GeometryUtils.AddBoundingBoxEdges(  
                obj, 
                ref bbox2, 
                Color.red, 
                0.01f, 
                layerInactive
            ); 
    } 


    public 
    void PhysicsRaycastUpdate(Camera camera) 
    {
        Vector3 mouse = Input.mousePosition;

        // X.0. 
        touched = null; 
        GeometryUtils.CameraPhysicsRaycastAdd(
            camera, 
            mouse.x, 
            mouse.y, 
            ref touched, 
            layerInactive);  

        foreach (Selected s in selections.Values) s.Reset(); 
        if(touched != null)
        {
            Selected s = new Selected(); 
            s.Touched(touched, bbox1, layerTouched); 
            if (!selections.ContainsKey(touched)) selections.Add(touched, s); 
        }
        chosen.DoSomething();
    }


    public 
    void RaycastOnMeshUpdate(Camera camera) 
    {
        Vector3 hitPoint = new Vector3(); 

        Vector3 mouse = Input.mousePosition;
        GeometryUtils.RaycastOnMesh(plane, camera,  mouse.x, mouse.y, ref hitPoint); 

        sphere.transform.position = hitPoint;
    }



    public
    void Init(GameObject obj)
    {
        //touched = null; 

        //bbox1 = new GameObject("bbox1"); 
        //bbox1.layer = layerInactive; 

        //bbox2 = new GameObject("bbox2"); 
        //bbox2.layer = layerInactive; 

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

        this.obj = obj; 
    }


    GameObject plane; 
    GameObject bbox1; 
    GameObject bbox2;    
    GameObject sphere;
    GameObject touched; 

    Chosen chosen = new Chosen(); 
    Dictionary<GameObject, Selected> selections = new Dictionary<GameObject, Selected>();
}


///////////////////////////////////////////////////////////////////////////////
public 
class CameraMain : MonoBehaviour
{
    public Camera cameraToControl;
    public GameObject fbxInstance;
/*
    GameObject plane; 
    GameObject bbox1; 
    GameObject bbox2;    
    GameObject sphere;
    GameObject touched; 
*/
    int layerTouched; 
    int layerInactive; 
    int layerRigidBodies; 

    ObjectMain objectMain = null; 


    void Start()
    {
        layerTouched = LayerMask.NameToLayer("Touched"); 
        layerInactive = LayerMask.NameToLayer("InActive"); 
        layerRigidBodies = LayerMask.NameToLayer("Lego"); 
/*
        GeometryUtils.GeometryControl(fbxInstance, 
            ref bbox1, ref bbox2, ref plane, ref sphere, 
            layerInactive, layerRigidBodies
        ); 
*/

        objectMain = gameObject.AddComponent<ObjectMain>(); 
        objectMain.layerTouched = layerTouched; 
        objectMain.layerInactive = layerInactive; 
        objectMain.layerRigidBodies = layerRigidBodies; 

        objectMain.Init(fbxInstance); 

        cameraControl.Init(fbxInstance, cameraToControl); 
    }


    void Update()
    {
//        GeometryUtils.AddBoundingBoxEdges(fbxInstance, bbox2, Color.red, 0.01f, layerInactive); 
        objectMain.Update(); 
/*
        // X.0. 
        Vector3 hitPoint = new Vector3(); 
        Vector3 mouse = Input.mousePosition;
        GeometryUtils.RaycastOnMesh(plane, cameraToControl,  mouse.x, mouse.y, ref hitPoint); 
*/
/*
        sphere.transform.position = hitPoint;

        // X.0. 
        touched = null; 
        GeometryUtils.CameraPhysicsRaycastAdd(cameraToControl, mouse.x, mouse.y, ref touched, layerInactive);  

        foreach (Selected s in selections.Values) s.Reset(); 
        if(touched != null)
        {
            Selected s = new Selected(); 
            s.Touched(touched, bbox1, layerTouched); 
            if (!selections.ContainsKey(touched)) selections.Add(touched, s); 
        }
        chosen.DoSomething();
*/

        objectMain.RaycastOnMeshUpdate(cameraToControl);
        objectMain.PhysicsRaycastUpdate(cameraToControl); 

        MouseClickCenter(); 
        MouseClickRight(); 
        MouseClickLeft(); 
    } 


    bool MouseClickCenter()
    {
        float scrollValue = Input.GetAxis("Mouse ScrollWheel");
        if( scrollValue > 0 ) cameraControl.Move(-1); 
        if( scrollValue < 0 ) cameraControl.Move( 1); 

        int bottom = (int) MouseButton.Center; 
        bool up = Input.GetMouseButtonUp(bottom); 
        bool down = Input.GetMouseButtonDown(bottom); 
        bool clicked = Input.GetMouseButton(bottom); 

        if (clicked) 
        {
//            chosen.CenterClick(touched, bbox1, layerInactive); 
        }

        return false; 
    }


    bool MouseClickRight() 
    { 
        int bottom = (int) MouseButton.Right; 
        bool up = Input.GetMouseButtonUp(bottom); 
        bool down = Input.GetMouseButtonDown(bottom); 
        bool clicked = Input.GetMouseButton(bottom); 

        if (up) {}

        if (down)
        {
            cameraControl.RightUp(); 
        }

        if (clicked) 
        {
            cameraControl.Move(0);
            return cameraControl.RightClick(); 
        }

        return false; 
    }


    bool MouseClickLeft() 
    { 
        int bottom = (int) MouseButton.Left; 
        bool up = Input.GetMouseButtonUp(bottom); 
        bool down = Input.GetMouseButtonDown(bottom); 
        bool clicked = Input.GetMouseButton(bottom); 

        if (up) {}

        if (down) {}

        if (clicked) 
        {
            cameraControl.Move(0);
            return cameraControl.LeftClick(); 
        }

        return false; 
    }


    public enum MouseButton{Left=0, Right=1, Center=2}

//    Chosen chosen = new Chosen(); 
    CameraControl cameraControl = new CameraControl(); 
//    Dictionary<GameObject, Selected> selections = new Dictionary<GameObject, Selected>();
}


///////////////////////////////////////////////////////////////////////////////