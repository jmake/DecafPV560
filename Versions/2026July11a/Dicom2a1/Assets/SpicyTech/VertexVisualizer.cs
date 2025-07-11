using System;
using System.Linq;
using System.Diagnostics;

using UnityEngine;


namespace SpicyTech {

[RequireComponent(typeof(MeshFilter), typeof(MeshRenderer))]
public class VertexVisualizer : MonoBehaviour
{
    [HideInInspector]
    public int[] triangles;      // length = nTriangles * 3

    [HideInInspector]
    public float[] vertices;     // length = nbVertices * verticesDims

    [HideInInspector]
    public float[] property;

    private Mesh mesh;

    int verticesDims = 3; 


    void Start()
    {
        if (vertices == null || vertices.Length % verticesDims != 0 || triangles == null || triangles.Length % 3 != 0)
        {
            UnityEngine.Debug.Log($"[VertexVisualizer] '{gameObject.name}' Invalid vertex or triangle data.");
            enabled = false;
            return;
        }

        mesh = new Mesh();
        // mesh index format is 16-bit, limiting a maximum of ~21,845 triangles (65,535/3)
        mesh.indexFormat = UnityEngine.Rendering.IndexFormat.UInt32;

        mesh.name = "GeneratedMesh";
        GetComponent<MeshFilter>().mesh = mesh;
        
        ExecutionTime(UpdateMesh , "[UpdateMesh] UnityMeshUpdate");
        MaterialCreate(); 
    }


    public void UpdateMesh()
    {
        if(
            vertices.Length % verticesDims != 0 || 
            triangles.Length % 3 != 0 || 
            property.Length <= 0 
        ) 
        return ; 

        if(
          triangles.Min() < 0 ||   
          triangles.Max() > property.Length
        )
        return ; 

        int nbVertices = vertices.Length / verticesDims;

        // nii.gz -> (a) ~10 ms, (b) ~100 ms
        if( mesh )
        {
            // (a) 
            Vector3[] meshVertices = new Vector3[nbVertices];
            for (int i = 0; i < nbVertices; i++)
            {
                meshVertices[i] = new Vector3(
                    vertices[i * verticesDims + 0],
                    vertices[i * verticesDims + 1],
                    vertices[i * verticesDims + 2]
                );
            }

            mesh.Clear();
            mesh.vertices = meshVertices;
            mesh.triangles = triangles;
            mesh.RecalculateNormals();

            // (b) 
            // MeshBuilder.CreateMeshFromRaw(ref mesh, vertices, nbVertices, triangles, triangles.Length); 

            // (c) extras ... 
            mesh.uv = Array2UV(); 
            mesh.RecalculateBounds();
            // // MaterialCreate(); 
        }
    }


    public Vector2[] Array2UV() 
    {
        Vector2[] uvs = new Vector2[property.Length];

        float min = property.Min();
        float max = property.Max();

        for (int i = 0; i < property.Length; i++)
        {
            float normalized = Mathf.InverseLerp(min, max, property[i]); // 0–1
            uvs[i] = new Vector2(normalized, 0); // Y can be zero, it's a 1D texture
        }

        return uvs;       
    }


    void MaterialCreate() 
    {
        var renderer = gameObject.GetComponent<MeshRenderer>();
        //MaterialSimplestGray(renderer.material); 
        renderer.material = MaterialCustom();  // :) 
    } 
    

    Material MaterialCustom()  
    {
        Material scalarMaterial = new Material(Shader.Find("Custom/ScalarColorShader"));

        Texture2D texture = GenerateGradientTexture(Color.black, Color.white); 
        scalarMaterial.SetTexture("_ColorMap", texture);
        return scalarMaterial;  
    }


    Texture2D GenerateGradientTexture(Color color1, Color color2, int width = 256)
    {
        Texture2D tex = new Texture2D(width, 1, TextureFormat.RGBA32, false);
        tex.wrapMode = TextureWrapMode.Clamp;

        for (int i = 0; i < width; i++)
        {
            float t = i / (float)(width - 1);
            Color color = Color.Lerp(color1, color2, t);
            tex.SetPixel(i, 0, color);
        }

        tex.Apply();
        return tex;
    }


    Material SimpleMaterialCreate() 
    {
        var material = new Material(Shader.Find("Unlit/Color"));
        material.color = Color.red;
        return material; 
    }


    public void ExecutionTime(Action action, string label)
    {
        Stopwatch stopwatch = Stopwatch.StartNew();
        action();
        stopwatch.Stop();

        float dtime = stopwatch.ElapsedMilliseconds / 1000.0f; 
        UnityEngine.Debug.Log($"{label} time : {dtime} seg ");
    }

} // VertexVisualizer


} // SpicyTech 

/*

    void MeshBuilderTest() 
    {
        float[] verts = { 0f, 0f, 0f, 1f, 0f, 0f, 0f, 1f, 0f };
        int[] indices = { 0, 1, 2 };
        
        Mesh m = null; 
        MeshBuilder.CreateMeshFromRaw(ref m, verts, 3, indices, 3);        
    }

*/