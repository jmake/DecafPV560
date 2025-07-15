using UnityEngine;
using System.Linq;

public class PixelTextureDisplay : MonoBehaviour
{
    int width;
    int height;

    public Color colorMin = Color.white;
    public Color colorMax = Color.black;
    public string id;

    GameObject plane = null;
    Texture2D texture = null;
    Material material = null;

    float lastScaleFactor = -1f;


    public void Init(int w, int h)
    {
        EnsureMaterial();
        CreateOrUpdateTexture(w, h);
        PlaneCreate();
        PlaneScaleUpdate(w, h, 1.0f);
    }


    public void PlaneUpdate(GameObject obj) 
    {
        plane.transform.rotation = obj.transform.rotation;
    }


    public void Apply(float[] data, float scaleFactor)
    {
        if (data == null) return;

        // Recreate texture if size mismatch
        if (data.Length != width * height || texture == null)
        {
            //int newWidth = width;
            //int newHeight = height;
            //CreateOrUpdateTexture(newWidth, newHeight);
            CreateOrUpdateTexture(width, height);
        }

        if (Mathf.Abs(scaleFactor - lastScaleFactor) > 0.001f) PlaneScaleUpdate(width, height, scaleFactor);

        if (data.Length != width * height) return;

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

    void EnsureMaterial()
    {
        if (material == null)
        {
            Shader shader = Shader.Find("Custom/UnlitTextureCullOff");
            material = new Material(shader);
        }
    }

    void CreateOrUpdateTexture(int newWidth, int newHeight)
    {
        Debug.Log($"[CreateOrUpdateTexture] newWidth : {newWidth} newHeight : {newHeight} ");

        if (texture == null || newWidth != width || newHeight != height)
        {
            width = newWidth;
            height = newHeight;

            if (texture != null) Destroy(texture);

            texture = new Texture2D(width, height);
            texture.filterMode = FilterMode.Point;

            if (plane != null)
            {
                plane.GetComponent<Renderer>().material.mainTexture = texture;
            }
        }
    }

    void PlaneCreate()
    {
        if (plane == null)
        {
            plane = GameObject.CreatePrimitive(PrimitiveType.Quad);
            plane.name = id;
            plane.GetComponent<Renderer>().material = material;
            plane.GetComponent<Renderer>().material.mainTexture = texture;

            plane.transform.SetParent(gameObject.transform); 
            //UpdateScale(scaleFactor);
        }
    }

    void PlaneScaleUpdate(int w, int h, float scaleFactor)
    {
        if (plane != null)
        {
            plane.transform.localScale = new Vector3(w * scaleFactor, h * scaleFactor, 1f);
            lastScaleFactor = scaleFactor;
        }
    }

    float[] GeneratePerlinData(int width, int height, float t)
    {
        float[] data = new float[width * height];
        for (int x = 0; x < width; x++)
            for (int y = 0; y < height; y++)
                data[x + y * width] = Mathf.PerlinNoise(x * 0.01f + t, y * 0.01f + t);
        return data;
    }
}


/*
using UnityEngine;
using System.Linq;


public class PixelTextureDisplay : MonoBehaviour
{
    int width; //= 256;
    int height; //= 512;

    //public float scaleFactor = 1f;
    public Color colorMin = Color.white; 
    public Color colorMax = Color.black; 
    public string id; 

    GameObject plane = null;

    Material material = null;
    Texture2D texture = null;

    float lastScaleFactor = -1f;


    public void Init(int width, int height)
    {
        EnsureMaterial();
        CreateTexture(width, height);
        CreatePlane(1.0f);
    }


    public void Apply(float[] data, float scaleFactor)
    {
        Apply(data, scaleFactor, colorMin, colorMax);
    }


    void Apply(float[] data, float scaleFactor, Color colorMax, Color colorMin)
    {
        if(texture == null) return ;

        if (Mathf.Abs(scaleFactor - lastScaleFactor) > 0.001f) UpdateScale(scaleFactor);

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


    void EnsureMaterial()
    {
        if (material == null)
        {
            Shader shader = Shader.Find("Unlit/Texture");
            material = new Material(shader);
        }
    }

    void CreateTexture(int width, int height)
    {
        this.width = width; 
        this.height = height; 

        if(texture == null)
        {
            texture = new Texture2D(width, height);
            texture.filterMode = FilterMode.Point;
        } 
    }

    void CreatePlane(float scaleFactor)
    {
        if(plane == null)
        {
            plane = GameObject.CreatePrimitive(PrimitiveType.Quad);
            plane.name = id; 
            plane.GetComponent<Renderer>().material = material;
            plane.GetComponent<Renderer>().material.mainTexture = texture;
            UpdateScale(scaleFactor);
        }
    }

    void UpdateScale(float scaleFactor)
    {
        if (plane != null)
        {
            plane.transform.localScale = new Vector3(width * scaleFactor, height * scaleFactor, 1f);
            lastScaleFactor = scaleFactor;
        }
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

}
*/