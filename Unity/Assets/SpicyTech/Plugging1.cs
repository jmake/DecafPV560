using System;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;

using System.IO;
using System.Collections.Generic;

using UnityEngine;

using SpicyTech;


public class Pluggin1 : MonoBehaviour
{
    Nifti nifti = new Nifti(); 

    public string niiFile;

    void Start()
    {
        string niiPath = Path.Combine(Application.streamingAssetsPath, niiFile);
        Debug.Log($"[Start] niiPath: '{niiPath}'");
        nifti.LoadFile(niiPath); 

        string vtiFile = "nifti"; 
        string vtiPath = Path.Combine(Application.streamingAssetsPath, vtiFile);
        nifti.VtiSave(vtiPath); 
        Debug.Log($"[Start] vtiPath: '{vtiPath}'");

        ContourCreate(nifti); 
        /*
        F:\z2025_1\ParaView600\bin\paraview.exe .\Dicom2a1\Assets\StreamingAssets\nifti.vti
        */
    }


    void Update()
    {
        
    }


    void ContourCreate(Nifti n) 
    {
        string contourFile = "contour_a"; 
        string contourPath = Path.Combine(Application.streamingAssetsPath, contourFile);
        n.ContourCreate("voxel"); 
        n.ContourSave(contourPath); 

        int[] dimensions = {0, 0, 0}; 
        n.ContourDimensions(dimensions); 

        int n_indices = dimensions[0] / 3; 
        int n_vertices = dimensions[1] / 3; 
        int n_property = dimensions[2]; 

        int[] indices = new int[dimensions[0]]; 
        float[] vertices = new float[dimensions[1]]; 
        float[] property = new float[dimensions[2]]; 
        n.ContourGeometry(indices, vertices, property); 

        GameObject go = new GameObject(contourFile);
        VertexVisualizer visualizer = go.AddComponent<VertexVisualizer>();

        visualizer.property = new float[property.Length];
        Array.Copy(property, visualizer.property, property.Length);

        visualizer.vertices = new float[vertices.Length];
        Array.Copy(vertices, visualizer.vertices, vertices.Length);

        visualizer.triangles = new int[indices.Length];
        Array.Copy( indices, visualizer.triangles, indices.Length);
    }




} // Pluggin1


/*
1) 
    Assests/Pluggins -> LibraryName.dll -> Inspector -> Load on startup -> Apply
2) 

*/