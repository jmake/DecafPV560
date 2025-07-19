using System;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;

using System.IO;
using System.Collections.Generic;

using UnityEngine;

using SpicyTech;


public class Plugging1 : MonoBehaviour
{    
    string niiFile = "BRATS_001.nii.gz"; // :) 
    //string niiFile = "1.2.826.0.1.3680043.10633.nii"; // :/ 

    Nifti nifti = null;  

    // contour
    VertexVisualizer visualizerCountour; 

    bool contourOn = false; 
    double maxCells = 2 * Math.Pow(2,20); // 2 * (1 048 576)
    public bool centered = true; 
    [Range(0.01f, 99.99f)] public float threshold = 50.0f;
    string contourName = "contour_a"; 
    float threshold_old; 

    // cutters
    bool cuttersOn = true; 
    List<VertexVisualizer> cutters; 

    public Vector3 center; 
    public Vector3 normal; 

    Vector3 center_old; 
    Vector3 normal_old; 

    void OnDisable()
    {
        if(nifti != null) 
        {
            //nifti.Finish(); 
            nifti = null; 
        }

        cuttersClear(); 
    }


    void cuttersClear()
    {
        if (cutters != null)
        {
            for (int i = 0; i < cutters.Count; i++)
            {
                if (cutters[i] != null) Destroy(cutters[i].gameObject);
            }

            cutters = null; 
        }
    }


    void Start()
    {
        if(nifti != null) return ; 

        nifti = new Nifti();

        string niiPath = Path.Combine(Application.streamingAssetsPath, niiFile);
        Debug.Log($"[Start] niiPath: '{niiPath}'");
        nifti.LoadFile(niiPath, centered); 

        if(contourOn)
        {
            nifti.ContourCreate("voxel", (float)maxCells); 
            visualizerCountour = VertexVisualizerCreate(nifti, contourName); 
            ContoursUpdate(nifti, visualizerCountour, threshold);
            threshold_old = threshold; 
        }

        if(cuttersOn)
        {
            center = new Vector3(); 
            normal = new Vector3(0.0f, 0.0f, 1.0f); 

            cutters = new List<VertexVisualizer>(); 
            nifti.CutCreate("voxel"); 
            VertexVisualizer cutter = VertexVisualizerCreate(nifti, "cutter1"); 
            CuttersUpdate(nifti, cutter, normal);  
            
            cutters.Add(cutter); 
            center_old = center; 
            normal_old = normal; 
        }

    }


    void FixedUpdate()
    {
        if(contourOn && threshold_old != threshold)
        {
            ContoursUpdate(nifti, visualizerCountour, threshold);
            Debug.Log($"[FixedUpdate] threshold: '{threshold}'");
            threshold_old = threshold; 
        }

        if(cuttersOn && normal_old != normal)
        {
            foreach (var cutter in cutters)
            {
                CuttersUpdate(nifti, cutter, normal);  
                Debug.Log($"[FixedUpdate] normal");

                center_old = center; 
                normal_old = normal; 
            }
        }
    }


    VertexVisualizer VertexVisualizerCreate(Nifti n, string name) 
    {
        GameObject go = new GameObject(name);
        return go.AddComponent<VertexVisualizer>();
    }


    void ContoursUpdate(Nifti n, VertexVisualizer vv, float value) 
    {
        n.ContourUpdate(value, 10.0f, 100.0f); 

        int[] dimensions = {0, 0, 0}; 
        n.ContourDimensions(dimensions); 

        int n_indices = dimensions[0] / 3; 
        int n_vertices = dimensions[1] / 3; 
        int n_property = dimensions[2]; 
        Debug.Log($"[Tester] n_indices:{n_indices} n_vertices:{n_vertices} n_property:{n_property} "); 

        if(n_indices >  maxCells) 
        {
            Debug.Log($"[Tester] n_indices:{n_indices} > {maxCells}"); 
            return; 
        }

        int[] triangles = new int[dimensions[0]]; 
        float[] vertices = new float[dimensions[1]]; 
        float[] property = new float[dimensions[2]]; 
        n.ContourGeometry(triangles, vertices, property); 
        //return ;

        vv.property = new float[property.Length];
        Array.Copy(property, vv.property, property.Length);

        vv.vertices = new float[vertices.Length];
        Array.Copy(vertices, vv.vertices, vertices.Length);

        vv.triangles = new int[triangles.Length];
        Array.Copy(triangles, vv.triangles, triangles.Length);

        vv.UpdateMesh(); 
    }


    void CuttersUpdate(Nifti n, VertexVisualizer vv, Vector3 normal) 
    {
        n.CutUpdate(0.0f,0.0f,0.0f, normal.x, normal.y, normal.z); 

        int[] dimensions = {0, 0, 0}; 
        n.CutDimensions(dimensions); 

        int n_indices = dimensions[0] / 3; 
        int n_vertices = dimensions[1] / 3; 
        int n_property = dimensions[2]; 
        Debug.Log($"[Tester] n_indices:{n_indices} n_vertices:{n_vertices} n_property:{n_property} "); 

        if(n_indices >  maxCells) 
        {
            Debug.Log($"[Tester] n_indices:{n_indices} > {maxCells}"); 
            return; 
        }

        int[] triangles = new int[dimensions[0]]; 
        float[] vertices = new float[dimensions[1]]; 
        float[] property = new float[dimensions[2]]; 
        n.CutGeometry(triangles, vertices, property); 
        //return ;

        vv.property = new float[property.Length];
        Array.Copy(property, vv.property, property.Length);

        vv.vertices = new float[vertices.Length];
        Array.Copy(vertices, vv.vertices, vertices.Length);

        vv.triangles = new int[triangles.Length];
        Array.Copy(triangles, vv.triangles, triangles.Length);

        vv.UpdateMesh(); 
    }


} // Pluggin1


/*
1) 
    Assests/Pluggins -> LibraryName.dll -> Inspector -> Load on startup -> Apply
2) 
    F:\z2025_1\ParaView600\bin\paraview.exe .\Dicom2a1\Assets\StreamingAssets\nifti.vti


*/

/*
Copy-Item -Recurse `
-Path F:\z2025_1\Dicom\DecafPV560\Unity\Assets\Plugins\* `
-Destination F:\z2025_1\Unity\Dicom2a1\Assets\Plugins\ 

*/