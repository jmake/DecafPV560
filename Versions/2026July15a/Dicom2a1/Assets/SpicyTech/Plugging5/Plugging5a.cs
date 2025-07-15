using System;
using System.IO;
using System.Linq;
using System.Text;
using System.Collections.Generic;
using System.Runtime.InteropServices;

using System.Security.Cryptography;
using UnityEngine;

using SpicyTech5;


//-----------------------------------------------------------------------//
//-----------------------------------------------------------------------//
public class Plugging5a : MonoBehaviour
{
    public bool cutter = true; 
    public bool contour = true; 

    public int selectedIndex = 0;

    public string[] fileOptions = {
        "1.2.826.0.1.3680043.10633.nii.gz",
        "BRATS_001.nii"
    };

    string selectedFile; 

    MainManager main; 
 

    void Start()
    {
        selectedFile = fileOptions[selectedIndex];
        Debug.Log($"selectedFile : '{selectedFile}'");

        main = gameObject.AddComponent<MainManager>(); 
        main.Init(selectedFile); 
    }


    void FixedUpdate()
    {
        main.Apply(cutter, contour); 
    }
}

//-----------------------------------------------------------------------//
//-----------------------------------------------------------------------//
namespace SpicyTech5 {


public class MainManager : MonoBehaviour
{
    [HideInInspector]
    public string message = ""; 

    bool isReady = false;
    NifftiManager manager = null; 

    Vector3    upPrevious; 
    Vector3 centerPrevious; 
    Vector3 normalPrevious; 

    bool cutterUpdate = false;
    GameObject cutterObj = null; 
    SpicyTech.CutterManager2 cutter = null; 
 

    double thresholdPrevious; 
    bool contourUpdate = false;
    GameObject contourObj = null; 
    SpicyTech.Contour2 contour = null; 

    Color colorMin = Color.black; 
    Color colorMax = Color.white; 


    void OnDisable()
    {
        if(cutter != null)
        {
            cutter.Finish(); 
            cutter = null; 
        }

        if(contour != null)
        {
            contour.Finish(); 
            contour = null; 
        }

        //if(manager != null) 
        manager.Finish(); 
        manager = null; 
    }


    public void Init(string niiFile)
    {
        string niiPath = Path.Combine(Application.streamingAssetsPath, niiFile);

        string message = $"Loading {niiFile} ...";

        manager = new NifftiManager(); 
/*
        isReady = false; 
        manager.Create(niiPath); 
        manager.PostProcess(); 
        isReady = true; 
*/
        
        gameObject.AddComponent<OnGUIThread5>().Apply(
            () => {
                isReady = false; 
                manager.Create(niiPath); 
            }, 
            message,  
            () => {
                manager.PostProcess(); 
                Debug.Log("[MainManager] Ready!"); 
                isReady = true; 
            }
        );
        
    }

    public
    void Apply(bool cutter, bool contour)
    {
        if(manager == null) return;

        if(!isReady) return;  

        if(cutter) CutterUpdate(); 

        if(contour) ContourUpdate(); 
    }


    void CutterUpdate()
    {
        if(cutterObj == null) 
        {
            cutterObj = new GameObject("Slicer1");
            CutterUI(cutterObj); 

            PixelTextureDisplay ptd = cutterObj.AddComponent<PixelTextureDisplay>();
            ptd.colorMin = colorMin; 
            ptd.colorMax = colorMax; 
            ptd.id = "Texture_" + cutterObj.name; 

            cutter = manager.CutterCreate(cutterObj);
            normalPrevious = new Vector3(); 
            centerPrevious = new Vector3(); 
        }

        Vector3     upCurrent = cutterObj.transform.up; 
        Vector3 normalCurrent = cutterObj.transform.forward; // -> (0,0,1) 
        Vector3 centerCurrent = cutterObj.transform.position;

        bool     upUpdate = (    upCurrent -     upPrevious).sqrMagnitude > 1e-3f; 
        bool normalUpdate = (normalCurrent - normalPrevious).sqrMagnitude > 1e-3f; 
        bool centerUpdate = (centerCurrent - centerPrevious).sqrMagnitude > 1e-3f; 
        cutterUpdate = normalUpdate || centerUpdate || upUpdate;

        if(cutter != null && cutterUpdate) 
        {
            Debug.Log("[cutterUpdate] ..."); 

            manager.CutterUpdate(cutterObj, cutter);
            upPrevious = upCurrent;
            normalPrevious = normalCurrent; 
            centerPrevious = centerCurrent;
        }
    } 


    void CutterUI(GameObject obj) 
    {
        // Z Rotation 
        SliderController sc1 = obj.AddComponent<SliderController>(); 
        sc1.PositionX = 0.5f; 
        sc1.PositionY = 0.89f; 
        sc1.SliderWidth = 0.2f; 
        sc1.SliderHeight = 0.02f; 

        sc1.min = (float) -90; 
        sc1.max = (float)  90;
        sc1.func = (input) => {
            Quaternion rot = Quaternion.Euler(input, 0f, 0f); // Rotate around X-axis :)
            Vector3 newForward = rot * Vector3.forward; 
            obj.transform.rotation = Quaternion.LookRotation(newForward, Vector3.up);
            //Quaternion rot = Quaternion.Euler(0f, 0f, input); // Rotate around Z-axis :)
            //Vector3 newUp = rot * Vector3.up;                 // rotated up vector
            //obj.transform.rotation = Quaternion.LookRotation(Vector3.forward, newUp);
        }; 

        // X Rotation 
        SliderController sc2 = obj.AddComponent<SliderController>(); 
        sc2.PositionX = 0.5f; 
        sc2.PositionY = 0.92f; 
        sc2.SliderWidth = 0.2f; 
        sc2.SliderHeight = 0.02f; 

        sc2.min = (float) -90; 
        sc2.max = (float)  90;
        sc2.func = (input) => {
            Quaternion rot = Quaternion.Euler(0f, input, 0f); // Rotate around Y-axis :) 
            //Quaternion rot = Quaternion.Euler(input, 0f, 0f); // Rotate around X-axis :)
            Vector3 newForward = rot * Vector3.forward; 
            obj.transform.rotation = Quaternion.LookRotation(newForward, Vector3.up);
        }; 

        // Z Translation 
        SliderController sc3 = obj.AddComponent<SliderController>(); 
        sc3.PositionX = 0.72f; 
        sc3.PositionY = 0.89f; 
        sc3.SliderWidth = 0.2f; 
        sc3.SliderHeight = 0.02f; 

        sc3.min = (float) manager.bounds[4]; 
        sc3.max = (float) manager.bounds[5];
        sc3.func = (input) => obj.transform.Translate(0, 0, input - obj.transform.position.z);

        // Y Translation 
        SliderController sc4 = obj.AddComponent<SliderController>(); 
        sc4.PositionX = 0.72f; 
        sc4.PositionY = 0.92f; 
        sc4.SliderWidth = 0.2f; 
        sc4.SliderHeight = 0.02f; 

        sc4.min = (float) manager.bounds[0]; 
        sc4.max = (float) manager.bounds[1];
        sc4.func = (input) => obj.transform.Translate(input - obj.transform.position.x, 0, 0);
    }


    void ContourUpdate()
    {
        if(contourObj == null) 
        {
            contourObj = new GameObject("Contour1");
            contourObj.AddComponent<MeshManager>().Init(); 
            contourObj.AddComponent<SliderController>(); 
            
            ButtonController bc = contourObj.AddComponent<ButtonController>(); 
            bc.title = "Contour"; 
            bc.func = (input) => contourObj.GetComponent<MeshRenderer>().enabled = input; 

            contour = manager.ContourCreate(contourObj);
            thresholdPrevious = 0.0; 
        }

        double thresholdCurrent = contourObj.GetComponent<SliderController>().value; 
        contourUpdate = Math.Abs(thresholdCurrent - thresholdPrevious) > 0.1f; 
        if(contour != null && contourUpdate) 
        {
            Debug.Log($"[contourUpdate] threshold : {thresholdPrevious} -> {thresholdCurrent}"); 

            Color color = ColorUpdate((float)thresholdCurrent, manager.range[0], manager.range[1]);
            manager.ContourUpdate(contourObj, contour, thresholdCurrent, color);
            thresholdPrevious = thresholdCurrent; 
        }
        
    } 


    Color ColorUpdate(float x, float minValue, float maxValue)
    {
        float u = Mathf.InverseLerp(minValue, maxValue, x);
        return Color.Lerp(colorMin, colorMax, u);
    }


} // MainManager 

} // SpicyTech5



//-----------------------------------------------------------------------//
namespace SpicyTech5 {

public class NifftiManager 
{
    bool loaded = false; 
    public SpicyTech.NifftiLoader nifti = null; 

    string key = "voxel"; 

    public float[] range = {0,0}; 
    public double[] bounds = {0,0,0, 0,0,0}; // xmin,xmax, ymin,ymax, zmin,zmax


    public void Finish()
    {
        if(nifti != null) 
        {
            nifti.Finish(); 
            nifti = null; 

            Debug.Log($"[OnDisable] 'nifti' removed !!");
        }
    }


    public void Create(string directory)
    {
        if(nifti != null) return; 

        nifti = new SpicyTech.NifftiLoader(); 

        bool centered = true; 
        loaded = nifti.LoadFile(directory, key, centered); 
        if(loaded) 
        {
            Debug.Log($"[NifftiManager] directory: '{directory}' loaded !!");

            nifti.RangeGet(range); // voxelMin,voxelMax. Slow :( 
            Debug.Log($"[NifftiManager] range:[{range[0]}, {range[1]}]"); 

            nifti.BoundsGet(bounds); // xmin,xmax, ymin,ymax, zmin,zmax
            Debug.Log($"[NifftiManager] bounds:[{bounds[0]},{bounds[1]}], [{bounds[2]},{bounds[3]}], [{bounds[4]},{bounds[5]}]"); 
        } 
        else
        {
            nifti = null; 
            Debug.Log($"[NifftiManager] directory: '{directory}' fail !!");
            return ; 
        }
    }


    public  
    SpicyTech.Contour2 ContourCreate(GameObject obj) 
    {
        SpicyTech.Contour2 contour = null; 

        if(nifti == null) return contour; 

        contour = VtkManager.ContourCreate(nifti); 
        return contour; 
    }


    public void ContourUpdate(GameObject obj, SpicyTech.Contour2 contour, double threshold, Color color) 
    {
        if(nifti == null) return ; 
        if(contour == null) return ; 

        VtkManager.ContourUpdate(contour, nifti, obj, threshold, color);  
        return ; 
    } 


    public  
    SpicyTech.CutterManager2 CutterCreate(GameObject obj) 
    {
        SpicyTech.CutterManager2 cutter = null; 

        if(nifti == null) return cutter; 

        cutter = VtkManager.CutterCreate(nifti); 
        return cutter; 
    }


    public bool CutterUpdate(GameObject obj, SpicyTech.CutterManager2 cutter) 
    {
        bool update = false; 
        if(nifti == null) return update; 
        if(cutter == null) return update; 

        Vector3 x = obj.transform.right;   // (1,0,0)
        Vector3 y = obj.transform.up;      // (0,1,0) 
        Vector3 z = obj.transform.forward; // (0,0,1) 
        Vector3 o = obj.transform.position;
        VtkManager.CutterUpdate(cutter, nifti, obj, x, y, z, o); 
        return update; 
    }


    public void PostProcess()
    {
        if(nifti == null) return ; 

        GameObject obj1 = new GameObject("WireframeCube");
        WireframeCube wfc = obj1.AddComponent<WireframeCube>(); 

        //bounds -> xmin,xmax, ymin,ymax, zmin,zmax
        wfc.xRange = new Vector2((float)bounds[0], (float)bounds[1]);
        wfc.yRange = new Vector2((float)bounds[2], (float)bounds[3]);
        wfc.zRange = new Vector2((float)bounds[4], (float)bounds[5]);
        wfc.lineWidth = 1.0f; 

        GameObject obj2 = new GameObject("CameraOrbit");
        CameraOrbit cor = obj2.AddComponent<CameraOrbit>(); 
        cor.cam = Camera.main; 
        cor.target = obj2.transform; 
    }

} // NifftiManager
} // SpicyTech5 


//-----------------------------------------------------------------------//
//-----------------------------------------------------------------------//
namespace SpicyTech5 {

public partial class VtkManager {

    static double maxCells = 2 * Math.Pow(2,20); // 2 * (1 048 576)

    static public 
    SpicyTech.CutterManager2 CutterCreate(
                                            SpicyTech.NifftiLoader nifti 
                                        ) 
    {
        SpicyTech.CutterManager2 cutter = new SpicyTech.CutterManager2();   // 
        cutter.VtiSet( nifti.GetObj(), "ImageScalars");                     // 
        return cutter; 
    }


    static public 
    void CutterUpdate(
                        SpicyTech.CutterManager2 cutter, 
                        SpicyTech.NifftiLoader nifti, 
                        GameObject obj, 
                        Vector3 x, 
                        Vector3 y, 
                        Vector3 z, 
                        Vector3 o
                    ) 
    {
        PixelTextureDisplay ptd = obj.GetComponent<PixelTextureDisplay>(); 
        TextureUpdate(cutter, nifti, ptd, x, y, z, o); 
        ptd.PlaneUpdate(obj); 
    }


    static public 
    bool TextureUpdate<T>(
                        T vtk, 
                        SpicyTech.NifftiLoader nifti, 
                        PixelTextureDisplay ptd, 
                        Vector3 x, 
                        Vector3 y, 
                        Vector3 z, 
                        Vector3 o, 
                        bool centered = true
                    ) where T : SpicyTech.CutterManager2
    {
        bool updated = false; 

        if(nifti == null) return updated; 

//Vector3 n0 = Vector3.Cross(x, y);
//Debug.Log($"[NifftiLoaderTest 3] normal : ({n0[0]},{n0[1]},{n0[2]}) "); 
//updated = vtk.Update(n0.x, n0.y, n0.z); 
        double[] axisX = {x[0],x[1],x[2]}; 
        double[] axisY = {y[0],y[1],y[2]}; 
        double[] axisZ = {z[0],z[1],z[2]}; 
        double[]  orig = {o[0],o[1],o[2]};
        updated = vtk.Update(axisX, axisY, axisZ, orig); 

        // Dimensions 
        double[] spacing = {0, 0, 0}; 
        vtk.GetSpacing(spacing); 
        Debug.Log($"[NifftiLoaderTest 3] spacing : ({spacing[0]},{spacing[1]},{spacing[2]}) "); 

        double[] origin = {0, 0, 0}; 
        vtk.GetOrigin(origin); 
        Debug.Log($"[NifftiLoaderTest 3] origin : ({origin[0]},{origin[1]},{origin[2]}) "); 

        int[] pointsByAxis = {0, 0, 0}; 
        int nPoints = vtk.PointsByAxis(pointsByAxis); 
        Debug.Log($"[NifftiLoaderTest 3] pointsbyAxis : ({pointsByAxis[0]},{pointsByAxis[1]},{pointsByAxis[2]}) -> {nPoints}"); 

        float[] property = new float[nPoints]; 
        vtk.GetProperty(property); 

        int id = property.Length-1;
        UnityEngine.Debug.Log($"[NifftiLoaderTest 3] id:{id}/{property.Length} property:{property[id]}"); 

        // PixelTextureDisplay
        ptd.Init(pointsByAxis[0], pointsByAxis[1]); 
        ptd.Apply(property, 0.175f); 

        return updated; 
    } // MeshUpdate

} // VtkManager
} // SpicyTech5


//-----------------------------------------------------------------------//
//-----------------------------------------------------------------------//
namespace SpicyTech5 {

public partial class VtkManager {

    static public 
    SpicyTech.Contour2 ContourCreate(
                                        SpicyTech.NifftiLoader nifti 
                                    ) 
    {
        string key = "voxel"; 

        float[] range = {0, 0}; 
        nifti.RangeGet(key, range); // Slow!!

        SpicyTech.Contour2 manager = new SpicyTech.Contour2(); // 
        manager.VtiSet(nifti.GetObj(), key, range, maxCells); // ~ 1.971 seg / 764550 cells (GetContour)
        return manager; 
    }


    static public 
    void ContourUpdate(
                        SpicyTech.Contour2 manager, 
                        SpicyTech.NifftiLoader nifti, 
                        GameObject obj, 
                        double threshold, 
                        Color color
                    ) 
    {
        MeshManager mm = obj.GetComponent<MeshManager>(); 
        obj.GetComponent<MeshRenderer>().material.SetColor("_Color", color); // :( 

        bool centered = true; 
        Action func = () => manager.Update(threshold,centered); 
        MeshUpdate(manager, nifti, mm, func); 
    }


    static public 
    bool MeshUpdate<T>(
                        T vtk, 
                        SpicyTech.NifftiLoader nifti, 
                        MeshManager mm, 
                        Action func
                    ) where T : SpicyTech.Contour2
    {
        if(nifti == null) return false; 

        func(); 

        int[] dimensions = {0, 0, 0}; 
        vtk.DimensionsGet(dimensions); 

        int n_indices = dimensions[0] / 3; 
        int n_vertices = dimensions[1] / 3; 
        int n_property = dimensions[2]; 
        Debug.Log($"[MeshUpdate] n_indices:{n_indices} n_vertices:{n_vertices} n_property:{n_property} "); 

        if(n_indices >  maxCells) 
        {
            Debug.Log($"[MeshUpdate] n_indices:{n_indices} > {maxCells}"); 
            return false; 
        }

        int[] triangles = new int[dimensions[0]]; 
        float[] vertices = new float[dimensions[1]]; 
        float[] property = new float[dimensions[2]]; 
        vtk.GeometryGet(triangles, vertices, property); 

        mm.Apply(triangles, vertices, property); 
        return true; 
    }


} // VtkManager

} // SpicyTech5



//-----------------------------------------------------------------------//





//-----------------------------------------------------------------------//
//-----------------------------------------------------------------------//