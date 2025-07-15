using System;
using System.Linq;
using System.Diagnostics;

using UnityEngine;


namespace SpicyTech5 {

[RequireComponent(typeof(MeshFilter), typeof(MeshRenderer))]
public class MeshManager : MonoBehaviour
{
    Mesh mesh = null;
    public Material material = null; 

    int verticesDims = 3; 
    int trianglesDims = 3; 

    public int n_indices; 
    public int n_vertices; 
    public int n_property; 


    public 
    void Init()
    {
        // [1] mesh index format is 16-bit, limiting a maximum of ~21,845 triangles (65,535/3)
        mesh = new Mesh();
        mesh.name = $"{gameObject.name}_Mesh"; 
        mesh.indexFormat = UnityEngine.Rendering.IndexFormat.UInt32; // [1]
        gameObject.GetComponent<MeshFilter>().mesh = mesh;

        material = gameObject.GetComponent<MeshRenderer>().material; 
        material.color = Color.green; 
        //Material material = new Material(Shader.Find("Custom/UnlitTextureCullOff")); 
        //gameObject.GetComponent<MeshRenderer>().material = material;
        //gameObject.GetComponent<MeshRenderer>().material = new Material(Shader.Find("Unlit/Color"));
        //gameObject.GetComponent<MeshRenderer>().material = MaterialCustom(); 

        n_indices = 0; 
        n_vertices = 0; 
        n_property = 0; 
    }


    public 
    void Apply(
        int[] triangles,  // -> nTriangles * trianglesDims
        float[] vertices, // -> nbVertices * verticesDims
        float[] property  // -> nbVertices 
    )
    {

        if(mesh == null) 
        {
            UnityEngine.Debug.Log($"[MeshManager] '{gameObject.name}' mesh == null!!"); 
            return ;
        } 

        if(vertices.Length % verticesDims != 0 || triangles.Length % trianglesDims != 0) 
        {
            UnityEngine.Debug.Log($"[MeshManager] Error 1 in '{gameObject.name}' !!"); 
            return ; 
        }

        if(triangles.Min() < 0 || triangles.Max() > triangles.Length / trianglesDims)
        {
            UnityEngine.Debug.Log($"[MeshManager] Error 2 in '{gameObject.name}' !!"); 
            return ; 
        }

        n_indices = triangles.Length / trianglesDims; 
        n_vertices = vertices.Length / verticesDims; 
        n_property = property.Length; 

        int nbVertices = vertices.Length / verticesDims;

        // nii.gz -> (a) ~10 ms, (b) ~100 ms
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
        mesh.RecalculateBounds();
    } // Apply


    void Array2UV(Mesh m, float[] property) 
    {
        if(property.Length <= 0) return ; 

        Vector2[] uvs = new Vector2[property.Length];

        float min = property.Min();
        float max = property.Max();

        for (int i = 0; i < property.Length; i++)
        {
            float normalized = Mathf.InverseLerp(min, max, property[i]); // 0–1
            uvs[i] = new Vector2(normalized, 0); // 'Y' can be zero, it's a 1D texture
        }

        m.uv = uvs; 
    }

/*
    void MaterialCreate() 
    {
        var renderer = gameObject.GetComponent<MeshRenderer>();
        //MaterialSimplestGray(renderer.material); 
        renderer.material = MaterialCustom();  // :) 
    } 
*/    

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

/*
    Material SimpleMaterialCreate() 
    {
        var material = new Material(Shader.Find("Unlit/Color"));
        material.color = Color.red;
        return material; 
    }
*/
/*
    public void ExecutionTime(Action action, string label)
    {
        Stopwatch stopwatch = Stopwatch.StartNew();
        action();
        stopwatch.Stop();

        float dtime = stopwatch.ElapsedMilliseconds / 1000.0f; 
        UnityEngine.Debug.Log($"{label} time : {dtime} seg ");
    }
*/
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