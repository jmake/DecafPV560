using UnityEngine;
using System.Linq;


public class PixelTextureDisplay : MonoBehaviour
{
    public int width = 256;
    public int height = 512;
    public float scaleFactor = 1f;

    public Color colorMin = Color.white; 
    public Color colorMax = Color.black; 

    Material material;
    GameObject plane;
    Texture2D texture;

    float lastScaleFactor = -1f;

/*
    void Start()
    {
        Init(); 
    }

    void Update()
    {
        float t = Time.time;
        float[] testData = GeneratePerlinData(width, height, t);
        Apply(testData); //, colorMin, colorMax);
    }
*/
    public void Init()
    {
        EnsureMaterial();
        CreateTexture();
        CreatePlane();
    }


    public void Apply(float[] data)
    {
        Apply(data, colorMin, colorMax);
    }


    void Apply(float[] data, Color colorMax, Color colorMin)
    {
        if (Mathf.Abs(scaleFactor - lastScaleFactor) > 0.001f)
        {
            UpdateScale();
        }

        if (data == null || data.Length != width * height) return;

        float minValue = data.Min();
        float maxValue = data.Max();

        Color[] pixels = new Color[width * height];

        for (int i = 0; i < data.Length; i++)
        {
            float t = Mathf.InverseLerp(minValue, maxValue, data[i]);
            pixels[i] = Color.Lerp(colorMin, colorMax, t);
        }

        texture.SetPixels(pixels);
        texture.Apply();
    }


    float[] GeneratePerlinData(int width, int height, float t)
    {
        float[] data = new float[width * height];
        for (int x = 0; x < width; x++)
        {
            for (int y = 0; y < height; y++)
            {
                float value = Mathf.PerlinNoise(x * 0.01f + t, y * 0.01f + t);
                data[x + y * width] = value;
            }
        }
        return data;
    }



/*
    public void Apply()
    {
        if (Mathf.Abs(scaleFactor - lastScaleFactor) > 0.001f)
        {
            UpdateScale();
        }

        // Animate pixel values
        Color[] pixels = new Color[width * height];
        float t = Time.time;
        for (int x = 0; x < width; x++)
        {
            for (int y = 0; y < height; y++)
            {
                float value = Mathf.PerlinNoise(x * 0.01f + t, y * 0.01f + t);
                pixels[x + y * width] = new Color(value, value, value);
            }
        }

        texture.SetPixels(pixels);
        texture.Apply();
    }
*/


    void EnsureMaterial()
    {
        if (material == null)
        {
            Shader shader = Shader.Find("Unlit/Texture");
            material = new Material(shader);
        }
    }

    void CreateTexture()
    {
        texture = new Texture2D(width, height);
        texture.filterMode = FilterMode.Point;
    }

    void CreatePlane()
    {
        plane = GameObject.CreatePrimitive(PrimitiveType.Quad);
        plane.GetComponent<Renderer>().material = material;
        plane.GetComponent<Renderer>().material.mainTexture = texture;
        UpdateScale();
    }

    void UpdateScale()
    {
        if (plane != null)
        {
            plane.transform.localScale = new Vector3(width * scaleFactor, height * scaleFactor, 1f);
            lastScaleFactor = scaleFactor;
        }
    }
}
