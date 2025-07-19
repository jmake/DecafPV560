using UnityEngine;
using UnityEngine.UI;


public class CustomRawImage : MonoBehaviour
{
    public RawImage rawImage;

    // Simulación de tu conjunto de valores
    private float[,] data = new float[1024, 1024];

    void Start()
    {
        // Simula datos (ruido o cualquier cosa que tú tengas)
        GenerateData();

        // Crear la textura
        Texture2D texture = GenerateTexture(data);
        texture.Apply();

        // Asignarla al RawImage
        rawImage.texture = texture;
    }

    void GenerateData()
    {
        for (int y = 0; y < 1024; y++)
        {
            for (int x = 0; x < 1024; x++)
            {
                // Ejemplo: ruido o algún cálculo
                data[x, y] = Mathf.PerlinNoise(x * 0.01f, y * 0.01f); // Valor entre 0 y 1
            }
        }
    }

    Texture2D GenerateTexture(float[,] values)
    {
        int width = values.GetLength(0);
        int height = values.GetLength(1);

        Texture2D texture = new Texture2D(width, height, TextureFormat.RGB24, false);
        texture.filterMode = FilterMode.Point; // O bilinear para suavizado

        Color[] pixels = new Color[width * height];

        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                float v = Mathf.Clamp01(values[x, y]);
                Color color = new Color(v, v, v); // Escala de grises
                pixels[y * width + x] = color;
            }
        }

        texture.SetPixels(pixels);
        return texture;
    }
}
