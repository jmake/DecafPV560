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


    void Update()
    {
        
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
    GameObject slicer = null; 
    NifftiManager manager = null; 


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
        gameObject.AddComponent<OnGUIThread5>().Apply(
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

} // SpicyTech5



//-----------------------------------------------------------------------//
namespace SpicyTech5 {

public class NifftiManager 
{
    bool loaded = false; 
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
} // SpicyTech5 


//-----------------------------------------------------------------------//
namespace SpicyTech5 {

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
        SpicyTech.CutterManager2 cutter = new SpicyTech.CutterManager2();   // 
        cutter.VtiSet( nifti.GetObj(), "ImageScalars");                     // 
        //cutter.Update(1.0f, 1.0f, 1.0f);                                    // 

        //SpicyTech.VertexVisualizer mesh = obj.AddComponent<SpicyTech.VertexVisualizer>();
        //MeshUpdate(cutter, nifti, mesh, normal); 

        PixelTextureDisplay ptd = obj.AddComponent<PixelTextureDisplay>();
        MeshUpdate(cutter, nifti, normal, ptd); 

        //mesh.UpdateMesh(); 
        ////cutter.Save(name); 
    }

    static public 
    bool MeshUpdate<T>(
                        T vtk, 
                        SpicyTech.NifftiLoader nifti, 
                        Vector3 n0, 
                        PixelTextureDisplay ptd, 
                        bool centered = true
                    ) where T : SpicyTech.CutterManager2
    {
        Debug.Log($"[NifftiLoaderTest 3] normal : ({n0[0]},{n0[1]},{n0[2]}) "); 

        bool updated = false; 

float dtime = 0.0f; 
System.Diagnostics.Stopwatch stopwatch1;  

        if(nifti == null) return updated; 

stopwatch1 = System.Diagnostics.Stopwatch.StartNew();

        updated = vtk.Update(1.0f, 1.0f, 1.0f); 
//        updated = vtk.Update(n0.x, n0.y, n0.z); 

stopwatch1.Stop();
dtime = stopwatch1.ElapsedMilliseconds / 1000.0f; 
UnityEngine.Debug.Log($"[MeshUpdate] vtkUpdate : {dtime} seg ");

        Debug.Log($"[MeshUpdate] updated : '{updated}' "); 
/*
        if(!updated) {
            Debug.Log($"[MeshUpdate] updated==false"); 
            return updated; 
        }
*/
stopwatch1 = System.Diagnostics.Stopwatch.StartNew();

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

/*
float[] data = null; //new float[width * height];

        // Accessing memory 
        unsafe {
            int[] Length = {0}; 
            ulong address = vtk.GetDataAddress(Length); // 0 seg
            IntPtr ptr = new IntPtr((long)address);  

if (address == 0 || Length[0] == 0)
{
    Debug.LogError("Invalid pointer or zero length.");
    return false;
}

data = new float[Length[0]];
//System.Runtime.InteropServices.Marshal.Copy(ptr, data, 0, Length[0]); // crash!!

            //float* property = (float*)ptr.ToPointer(); // unmanaged pointer
            //Debug.Log($"[NifftiLoaderTest 3] Length : {Length[0]} "); 
//Debug.Log($"[NifftiLoaderTest 3] property : {property[0]}");  // :( 
        } // unsafe
*/

    float[] property = new float[nPoints]; 
    vtk.GetProperty(property); 

int id = property.Length-1;
UnityEngine.Debug.Log($"[NifftiLoaderTest 3] id:{id}/{property.Length} property:{property[id]}"); 


stopwatch1.Stop();
dtime = stopwatch1.ElapsedMilliseconds / 1000.0f; 
UnityEngine.Debug.Log($"[MeshUpdate] vtkGetDataAddress : {dtime} seg ");


stopwatch1 = System.Diagnostics.Stopwatch.StartNew();

    ptd.width = pointsByAxis[0]; 
    ptd.height = pointsByAxis[1]; 
    ptd.scaleFactor = 0.175f; 
    ptd.Init(); 
    ptd.Apply( property ); 

stopwatch1.Stop();
dtime = stopwatch1.ElapsedMilliseconds / 1000.0f; 
UnityEngine.Debug.Log($"[MeshUpdate] PixelTextureDisplay : {dtime} seg ");


        return updated; 
    } // MeshUpdate

} // VtkManager
} // SpicyTech5


//-----------------------------------------------------------------------//
namespace SpicyTech5 {



} // SpicyTech5



//-----------------------------------------------------------------------//





//-----------------------------------------------------------------------//
//-----------------------------------------------------------------------//