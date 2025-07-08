using System;
using System.IO;
using System.Linq;
using System.Text;
using System.Collections.Generic;
using System.Runtime.InteropServices;

using System.Security.Cryptography;

using SpicyTech;
using UnityEngine;



namespace SpicyTech {


public class SpicyManager : NifftiManager 
{
    Dictionary<string, GameObject> slicers = null;
    Dictionary<string, CutterManager> cutters = null;
    Dictionary<string, CubeController> cubeControllers = null;

    Vector3 normalNew; 
    Vector3 normalPrevious; 

    public new void Finish()
    {
        base.Finish(); 

        if (cutters != null)
        {
            foreach (var pair in cutters)
            {
                pair.Value.Finish();
            }
            cutters = null;
        }

        if (cubeControllers != null)
        {
            foreach (var pair in cubeControllers)
            {
                pair.Value.Finish();
            }
            cubeControllers = null;
        }
    }


    public void CuttersFixedUpdate()
    {
        if(cutters == null) return ; 

        foreach (var pair in cubeControllers)
        {
            var controller = pair.Value;
            bool moving = controller.Apply(); 

            var key = pair.Key;
            GameObject slicer = slicers[key];

            //Debug.Log($"[CuttersFixedUpdate] moving : '{ moving }' '{controller.normalCurrent}' '{controller.normalPrevious}'");
            bool updateNormal = (controller.normalPrevious - controller.normalCurrent).sqrMagnitude > 1e-6f; 

slicer.SetActive(!moving);
controller.plane.SetActive(moving);

            if(!moving && updateNormal)
            {
                Debug.Log($"[CuttersFixedUpdate] moving : '{controller.normalCurrent}' -> '{controller.normalPrevious}'");

                CutterManager cutter = cutters[key];
                VertexVisualizer mesh = slicer.GetComponent<VertexVisualizer>();

                //MeshUpdate(cutter, mesh, controller.normalCurrent); // :) 
                //mesh.UpdateMesh();                                  // :) 

//slicer.SetActive(false);
//controller.plane.SetActive(true);

                slicer.AddComponent<OnGUIThread>().Apply(
                    () => {
                        MeshUpdate(cutter, mesh, controller.normalCurrent); // this could crash because the 'UpdateMesh'!!
                    }, 
                    "Uploading ... ", 
                    () => {
                        mesh.UpdateMesh(); // this could crash but can not run inside thread!!
//controller.plane.SetActive(false);
//slicer.SetActive(true);
                    }, 
                    () => {
//controller.plane.SetActive(true);
//slicer.SetActive(false);
                    }
                ); 

                controller.normalPrevious = controller.normalCurrent; 
            }
            else
            {
//slicer.SetActive(true);
//controller.plane.SetActive(moving);
            }

        }
    } 


    public void CuttersCreate()
    {
        if(nifti == null) return ; 

        if(cutters != null) return ;

        slicers = new Dictionary<string, GameObject>();
        cutters = new Dictionary<string, CutterManager>();
        cubeControllers = new Dictionary<string, CubeController>();

        CuttersAdd("xyz");

        Debug.Log($"[SpicyManager] cutters created !!");
    }


    public CutterManager CuttersAdd(string name)
    {
        // 1.0) Controller (Unity Mesh)
        GameObject slicer = new GameObject(name+"_Slicer");
        slicer.transform.position = new Vector3(0, 0.0f, 0);
        slicer.transform.localRotation = Quaternion.Euler(0,0,0); 

        // 1.1) slicer + controller (cube) + reference (plane)
        CubeController cubeController = slicer.AddComponent<CubeController>(); 
        cubeController.Create(name, Quaternion.Euler(0,40,0) ); 
        //slicer.transform.SetParent( cubeController.cube.transform, true); 
        //cubeController.normalPrevious = cubeController.normalCurrent; 

        // 2.0) Vtk Mesh 
        CutterManager cutter = new CutterManager(); 
        cutter.VtiSet( nifti.GetObj(), key); 

        VertexVisualizer mesh = slicer.AddComponent<VertexVisualizer>();
        MeshUpdate(cutter, mesh, cubeController.normalCurrent); 
        mesh.UpdateMesh(); //cutter.Save(name); 

        slicers[name] = slicer;
        cutters[name] = cutter;
        cubeControllers[name] = cubeController; 
        return cutter; 
    }

} // SpicyManager


public class NifftiManager 
{
    bool loaded = false; 
    bool updated = false; 

    double maxCells = 2 * Math.Pow(2,20); // 2 * (1 048 576)

    public string key = "voxel"; 
    public NifftiLoader nifti = null; 


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
        nifti = new NifftiLoader(); 

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


    public bool MeshUpdate<T>(T vtk, VertexVisualizer vv, Vector3 n0, bool centered = true) where T : CutterManager
    {
        updated = false; 

        updated = vtk.Update(n0.x, n0.y, n0.z, centered); 
        Debug.Log($"[CutterUpdate] updated : '{updated}' "); 

        if(!updated) {
            Debug.Log($"[CutterUpdate] updated==false"); 
            return updated; 
        }

        int[] dimensions = {0, 0, 0}; 
        vtk.DimensionsGet(dimensions); 

        int n_indices = dimensions[0] / 3; 
        int n_vertices = dimensions[1] / 3; 
        int n_property = dimensions[2]; 
        Debug.Log($"[CutterUpdate] n_indices:{n_indices} n_vertices:{n_vertices} n_property:{n_property} "); 

        if(n_indices >  maxCells) 
        {
            Debug.Log($"[CutterUpdate] n_indices:{n_indices} > {maxCells}"); 
            return updated; 
        }

        int[] triangles = new int[dimensions[0]]; 
        float[] vertices = new float[dimensions[1]]; 
        float[] property = new float[dimensions[2]]; 
        vtk.GeometryGet(triangles, vertices, property); 

        vv.property = new float[property.Length];
        Array.Copy(property, vv.property, property.Length);

        vv.vertices = new float[vertices.Length];
        Array.Copy(vertices, vv.vertices, vertices.Length);

        vv.triangles = new int[triangles.Length];
        Array.Copy(triangles, vv.triangles, triangles.Length);

        // Error : 'UnityException: must be called from the main thread'
        // It works well here in general except in the case of threading 
        //vv.UpdateMesh(); 
        return updated; 
    }

} // NifftiManager

} // SpicyTech


public class Plugging3 : MonoBehaviour
{
    // n_indices:1_032_004 n_vertices:517_448, ~ 21 segs 
    string niiFile = "1.2.826.0.1.3680043.10633.nii.gz"; 


    void OnDisable()
    {
        manager.Finish(); 
        manager = null; 
    }


    void FixedUpdate()
    {
        if (!isReady) return;

        manager.CuttersCreate(); 
        manager.CuttersFixedUpdate(); 
    } 


    void Start()
    {
        gameObject.AddComponent<OnGUIThread>().Apply(
            this.Init, 
            "Loading file ...", 
            () => Debug.Log("Done!")
        ); 
    }


    //System.Collections.IEnumerator 
    void Init()
    {
        //  n_indices:61_754 n_vertices:31_194, ~ 1.6 segs  
        niiFile = "BRATS_001.nii"; 
        
        string niiPath = Path.Combine(Application.streamingAssetsPath, niiFile);

        manager = new SpicyManager(); 
        manager.Create(niiPath); 

        //yield return null;
        isReady = true; 
    }


    bool isReady = false;
    SpicyManager manager = null; 
}