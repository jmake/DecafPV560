using UnityEngine;
using System; 

namespace SpicyTech5 {
//-----------------------------------------------------------------------//

public class SliderController : MonoBehaviour
{
    // Screen-relative slider layout
    [Range(0f, 1f)] public float PositionX = 0.01f;
    [Range(0f, 1f)] public float PositionY = 0.005f;
    [Range(0.01f, 1f)] public float SliderWidth = 0.98f;
    [Range(0.01f, 1f)] public float SliderHeight = 0.05f;

    public float titleSize = 0.5f; 
    public float titlePosition = 0.01f; 

    public float thumbSize = 1.0f; 
    public float trackSize = 1.0f; 

    public string message = ""; 

    public float min = 0.0f; 
    public float max = 1.0f;

    public float value = 0.5f; 
    public float valuePrevious = 0.5f;

    public Action<float> func;


    void FixedUpdate()
    {
        // Example input control (optional)
        if (Input.GetKey(KeyCode.W)) PositionY -= 0.005f;
        if (Input.GetKey(KeyCode.S)) PositionY += 0.005f;
        if (Input.GetKey(KeyCode.A)) PositionX -= 0.005f;
        if (Input.GetKey(KeyCode.D)) PositionX += 0.005f;

        PositionX = Mathf.Clamp01(PositionX);
        PositionY = Mathf.Clamp01(PositionY);
    }


    void OnGUI()
    {
        DrawScreenSlider(PositionX, PositionY, SliderWidth, SliderHeight);

        if(Math.Abs(value - valuePrevious) > 0.01) 
        {
            Debug.Log($"[SliderController] value : {valuePrevious} -> {value}");

            func?.Invoke(value);
            valuePrevious = value;
        }
    }


    void DrawScreenSlider(float xP, float yP, float wP, float hP)
    {
        float x = Screen.width * Mathf.Clamp01(xP);
        float y = Screen.height * Mathf.Clamp01(yP);
        float width = Screen.width * Mathf.Clamp01(wP);
        float height = Screen.height * Mathf.Clamp01(hP);

        GUIStyle labelStyle = new GUIStyle(GUI.skin.label)
        {
            alignment = TextAnchor.MiddleCenter,
            fontSize = Mathf.RoundToInt(height * titleSize),
            normal = { textColor = Color.white }
        };

        GUI.Label(new Rect(x, y - height * titlePosition, width, height), $"{message}{value:F2}", labelStyle);

        GUIStyle thumbStyle = new GUIStyle(GUI.skin.horizontalSliderThumb);
        GUIStyle sliderStyle = new GUIStyle(GUI.skin.horizontalSlider);

        float trackHeight = height * trackSize; 
        sliderStyle.fixedHeight = trackHeight;

        thumbStyle.fixedWidth = trackHeight * thumbSize;
        thumbStyle.fixedHeight = trackHeight * thumbSize; 
        value = GUI.HorizontalSlider(new Rect(x, y, width, height), value, min, max, sliderStyle, thumbStyle);
    }

} // SliderController

} // SpicyTech5 
//-----------------------------------------------------------------------//