using System;
using System.IO;
using System.Linq;
using System.Text;
using System.Collections.Generic;
using System.Runtime.InteropServices;

using System.Security.Cryptography;

//using SpicyTech;
using SpicyTech4;

using UnityEngine;


//-----------------------------------------------------------------------//
//-----------------------------------------------------------------------//
namespace SpicyTech4 {



} // SpicyTech4


//-----------------------------------------------------------------------//
//-----------------------------------------------------------------------//
namespace SpicyTech4 {

public class VtkManager {

    static double maxCells = 2 * Math.Pow(2,20); // 2 * (1 048 576)

    static public 
    void CutterCreate(
                        SpicyTech.NifftiLoader nifti, 
                        GameObject obj, 
                        string key, 
                        Vector3 normal
                    ) 
    {
        SpicyTech.CutterManager cutter = new SpicyTech.CutterManager(); 
        cutter.VtiSet( nifti.GetObj(), key); 

        SpicyTech.VertexVisualizer mesh = obj.AddComponent<SpicyTech.VertexVisualizer>();
        MeshUpdate(cutter, nifti, mesh, normal); 
        mesh.UpdateMesh(); 
        //cutter.Save(name); 
    }


    static public 
    bool MeshUpdate<T>(
                        T vtk, 
                        SpicyTech.NifftiLoader nifti, 
                        SpicyTech.VertexVisualizer vv, 
                        Vector3 n0, 
                        bool centered = true
                    ) where T : SpicyTech.CutterManager
    {
        bool updated = false; 

float dtime = 0.0f; 
System.Diagnostics.Stopwatch stopwatch1;  

        if(nifti == null) return updated; 


stopwatch1 = System.Diagnostics.Stopwatch.StartNew();
        updated = vtk.Update(n0.x, n0.y, n0.z, centered); 
stopwatch1.Stop();
dtime = stopwatch1.ElapsedMilliseconds / 1000.0f; 
UnityEngine.Debug.Log($"[MeshUpdate] vtkUpdate : {dtime} seg ");

        Debug.Log($"[MeshUpdate] updated : '{updated}' "); 

        if(!updated) {
            Debug.Log($"[MeshUpdate] updated==false"); 
            return updated; 
        }

        int[] dimensions = {0, 0, 0}; 
        vtk.DimensionsGet(dimensions); 

        int n_indices = dimensions[0] / 3; 
        int n_vertices = dimensions[1] / 3; 
        int n_property = dimensions[2]; 
        Debug.Log($"[MeshUpdate] n_indices:{n_indices} n_vertices:{n_vertices} n_property:{n_property} "); 

        if(n_indices >  maxCells) 
        {
            Debug.Log($"[MeshUpdate] n_indices:{n_indices} > {maxCells}"); 
            return updated; 
        }

stopwatch1 = System.Diagnostics.Stopwatch.StartNew();
        int[] triangles = new int[dimensions[0]]; 
        float[] vertices = new float[dimensions[1]]; 
        float[] property = new float[dimensions[2]]; 
        vtk.GeometryGet(triangles, vertices, property); 
stopwatch1.Stop();
dtime = stopwatch1.ElapsedMilliseconds / 1000.0f; 
UnityEngine.Debug.Log($"[MeshUpdate] vtkGeometryGet : {dtime} seg ");


stopwatch1 = System.Diagnostics.Stopwatch.StartNew();
        vv.property = new float[property.Length];
        Array.Copy(property, vv.property, property.Length);

        vv.vertices = new float[vertices.Length];
        Array.Copy(vertices, vv.vertices, vertices.Length);

        vv.triangles = new int[triangles.Length];
        Array.Copy(triangles, vv.triangles, triangles.Length);
stopwatch1.Stop();
dtime = stopwatch1.ElapsedMilliseconds / 1000.0f; 
UnityEngine.Debug.Log($"[MeshUpdate] vtk2UnityMesh : {dtime} seg ");

        // 
        // Error : 'UnityException: must be called from the main thread'
        // It works well here in general except in the case of threading 
        // vv.UpdateMesh();
        // 
        return updated; 
    }

} // VtkManager

} // SpicyTech4


namespace SpicyTech4 {

public class NifftiManager 
{
    bool loaded = false; 
    //bool updated = false; 

    //double maxCells = 2 * Math.Pow(2,20); // 2 * (1 048 576)

    public string key = "voxel"; 
    public SpicyTech.NifftiLoader nifti = null; 


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
        } 
        else
        {
            nifti = null; 
            Debug.Log($"[NifftiManager] directory: '{directory}' fail !!");
            return ; 
        }

    }


    public void CutterCreate(GameObject obj) 
    {
        if(nifti == null) return; 

        //GameObject slicer = new GameObject(name+"_Slicer");
        Vector3 normal = obj.transform.forward; 

        VtkManager.CutterCreate(
                                nifti, 
                                obj, 
                                key, 
                                normal
                            ); 
    }


} // NifftiManager


} // SpicyTech4


//-----------------------------------------------------------------------//
//-----------------------------------------------------------------------//
namespace SpicyTech4 {


public class MainManager : MonoBehaviour
{
    bool isReady = false;

    //SpicyTech.SpicyManager 
    NifftiManager manager = null; 
    
    [HideInInspector]
    public string message = ""; 

    GameObject slicer = null; 


    void OnDisable()
    {
        manager.Finish(); 
        manager = null; 
    }


    void FixedUpdate()
    {
        if(manager == null) return;

        if(!isReady) return;  

        if(slicer == null) 
        {
            slicer = new GameObject("forward_Slicer");
            manager.CutterCreate(slicer);
        }

    } 


    public void Init(string niiFile)
    {        
        string niiPath = Path.Combine(Application.streamingAssetsPath, niiFile);

        string message = $"Loading {niiFile} ...";

        manager = new NifftiManager(); 
        gameObject.AddComponent<OnGUIThread4>().Apply(
            () => {
                isReady = false; 
                manager.Create(niiPath); 
            }, 
            message,  
            () => {
                Debug.Log("[MainManager] Ready!"); 
                isReady = true; 
            }
        );
    }


} // MainManager 


} // SpicyTech4


//-----------------------------------------------------------------------//
//-----------------------------------------------------------------------//
public class Plugging4 : MonoBehaviour
{
    public int selectedIndex = 0;

    public string[] fileOptions = {
        "1.2.826.0.1.3680043.10633.nii.gz",
        "BRATS_001.nii"
    };

    string selectedFile; 

    MainManager main; 
    string message; 


    void Start()
    {
        message = "starting...";

        selectedFile = fileOptions[selectedIndex];
        Debug.Log( selectedFile );         

        main = gameObject.AddComponent<MainManager>(); 
        main.Init(selectedFile); 

        message = main.message;
    }


    void Update()
    {

    }


    void OnGUI()
    {
        DrawCenteredLabel(message, 0.15f, 0.1f); 
    }


    //public  
    void DrawCenteredLabel(string text, float widthPercent, float heightPercent)
    {
        // Clamp between 0 and 1
        widthPercent = Mathf.Clamp01(widthPercent);
        heightPercent = Mathf.Clamp01(heightPercent);

        float width = Screen.width * widthPercent;
        float height = Screen.height * heightPercent;

        float x = (Screen.width - width);
        float y = (Screen.height - height);

        GUIStyle style = new GUIStyle(GUI.skin.label);
        style.alignment = TextAnchor.MiddleCenter;
        style.fontSize = Mathf.RoundToInt(height * 0.3f); // scalable font size

        GUI.Label(new Rect(x, y, width, height), text, style);
    }


} // Plugging4