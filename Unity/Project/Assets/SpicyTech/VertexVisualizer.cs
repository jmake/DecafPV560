using System.Linq;

using UnityEngine;


[RequireComponent(typeof(MeshFilter), typeof(MeshRenderer))]
public class VertexVisualizer : MonoBehaviour
{
    public int[] triangles;      // length = nTriangles * 3
    public float[] vertices;     // length = nbVertices * verticesDims
    public float[] property;

    private Mesh mesh;

    int verticesDims = 3; 
    void Start()
    {
        if (vertices == null || vertices.Length % verticesDims != 0 || triangles == null || triangles.Length % 3 != 0)
        {
            Debug.LogError("Invalid vertex or triangle data.");
            enabled = false;
            return;
        }

        mesh = new Mesh();
        mesh.name = "GeneratedMesh";
        GetComponent<MeshFilter>().mesh = mesh;
        
        UpdateMesh();
    }


    void FixedUpdate()
    {
        UpdateMesh(); // Updates vertex positions each physics step
    }


    void UpdateMesh()
    {
        int nbVertices = vertices.Length / verticesDims;

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

        Array2UV(); 
        MaterialCreate(); 
    }


    public void Array2UV() 
    {
        Vector2[] uvs = new Vector2[property.Length];

        float min = property.Min();
        float max = property.Max();

        for (int i = 0; i < property.Length; i++)
        {
            float normalized = Mathf.InverseLerp(min, max, property[i]); // 0–1
            uvs[i] = new Vector2(normalized, 0); // Y can be zero, it's a 1D texture
        }

        mesh.uv = uvs;        
    }


    void MaterialCreate() 
    {
        var renderer = gameObject.GetComponent<MeshRenderer>();
        MaterialSimplestGray(renderer.material); 
        //renderer.material = MaterialCustom();  :) 
    } 
    

    void MaterialSimplestGray(Material material) 
    {
        Texture2D colormapTex = GenerateGradientTexture();
        material.SetTexture("_ColorMap", colormapTex);
    }


    Material MaterialCustom()  
    {
        Material scalarMaterial = new Material(Shader.Find("Custom/ScalarColorShader"));
        scalarMaterial.SetTexture("_ColorMap", GenerateGradientTexture());
        return scalarMaterial;  
    }


    Texture2D GenerateGradientTexture(int width = 256)
    {
        Texture2D tex = new Texture2D(width, 1, TextureFormat.RGBA32, false);
        tex.wrapMode = TextureWrapMode.Clamp;

        for (int i = 0; i < width; i++)
        {
            float t = i / (float)(width - 1);
            Color color = Color.Lerp(Color.blue, Color.red, t); // Simple blue-to-red gradient
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

}