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

    SpicyTech.CutterManager2 cutter = null; 

    bool updateCutter = false;
    Vector3 normalPrevious; 


    void OnDisable()
    {
        if(cutter != null)
        {
            cutter.Finish(); 
            cutter = null; 
        }

        //if(manager != null) 
        manager.Finish(); 
        manager = null; 
    }


    void FixedUpdate()
    {
        if(manager == null) return;

        if(!isReady) return;  

        if(slicer == null) 
        {
            slicer = new GameObject("Slicer1");

            PixelTextureDisplay ptd = slicer.AddComponent<PixelTextureDisplay>();
            ptd.colorMin = Color.black; 
            ptd.colorMax = Color.white; 
            ptd.id = "Texture_" + slicer.name; 

            cutter = manager.CutterCreate(slicer);
            normalPrevious = new Vector3(); 
        }

        updateCutter = (slicer.transform.forward - normalPrevious).sqrMagnitude > 1e-3f; 
        if(cutter != null && updateCutter) 
        {
            Debug.Log("[FixedUpdate] ..."); 

            manager.CutterUpdate(slicer, cutter);
            normalPrevious = slicer.transform.forward; // (0,0,1) 
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

float[] range = {0, 0}; 
nifti.RangeGet(range); 
Debug.Log($"[NifftiManager] range:[{range[0]}, {range[1]}]"); 

        } 
        else
        {
            nifti = null; 
            Debug.Log($"[NifftiManager] directory: '{directory}' fail !!");
            return ; 
        }
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
        bool update = false ; 

        if(nifti == null) return update; 
        if(cutter == null) return update; 

        Vector3 x = obj.transform.right;   // (1,0,0)
        Vector3 y = obj.transform.up;      // (0,1,0) 
        VtkManager.CutterUpdate(cutter, nifti, obj, x, y); 
        return update; 
    }


} // NifftiManager
} // SpicyTech5 


//-----------------------------------------------------------------------//
namespace SpicyTech5 {

public class VtkManager {

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
                        Vector3 y  
                    ) 
    {
        PixelTextureDisplay ptd = obj.GetComponent<PixelTextureDisplay>(); 
        TextureUpdate(cutter, nifti, ptd, x, y); 
        ptd.PlaneUpdate(obj); 
    }


    static public 
    bool TextureUpdate<T>(
                        T vtk, 
                        SpicyTech.NifftiLoader nifti, 
                        PixelTextureDisplay ptd, 
                        Vector3 x, 
                        Vector3 y, 
                        bool centered = true
                    ) where T : SpicyTech.CutterManager2
    {
        bool updated = false; 

        if(nifti == null) return updated; 

        Vector3 n0 = Vector3.Cross(x, y);
        Debug.Log($"[NifftiLoaderTest 3] normal : ({n0[0]},{n0[1]},{n0[2]}) "); 

        updated = vtk.Update(n0.x, n0.y, n0.z); 

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
namespace SpicyTech5 {



} // SpicyTech5



//-----------------------------------------------------------------------//





//-----------------------------------------------------------------------//
//-----------------------------------------------------------------------//