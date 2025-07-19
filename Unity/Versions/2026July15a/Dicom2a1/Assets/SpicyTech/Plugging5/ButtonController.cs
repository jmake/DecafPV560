using System;
using UnityEngine;


namespace SpicyTech5 {
//-----------------------------------------------------------------------//

public class ButtonController : MonoBehaviour
{
    // Toogle
    [Range(0f, 1f)] public float PositionX = 0.01f;
    [Range(0f, 1f)] public float PositionY = 0.94f;

    [Range(0.01f, 1f)] public float SliderWidth = 0.1f;
    [Range(0.01f, 1f)] public float SliderHeight = 0.05f;

    [Range(0.01f, 1f)] public float titleSize = 0.65f; 

    public bool toggleState = true;
    public string labelOn = "On"; 
    public string labelOff = "Off"; 

    // Title 
    public string title = "Title"; 
    public float titlePosition = 0.9f; 

    public Action<bool> func;

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
        bool newState = DrawScreenToggle(PositionX, PositionY, SliderWidth, SliderHeight, toggleState);

        if (newState != toggleState)
        {
            func?.Invoke(newState);
            /*
            if (toggleState) Debug.Log("Activated!");
            else Debug.Log("Deactivated!");
            */
            toggleState = newState;
        }
    }


    bool DrawScreenToggle(
        float xP, float yP, float wP, float hP,
        bool currentState)
    {
        float x = Screen.width * Mathf.Clamp01(xP);
        float y = Screen.height * Mathf.Clamp01(yP);
        float width = Screen.width * Mathf.Clamp01(wP);
        float height = Screen.height * Mathf.Clamp01(hP);

        DrawScreenTitle(x, y, width, height, title);  

        // Toogle
        Color originalColor = GUI.color;
        GUI.color = new Color(0.5f, 0.5f, 0.5f, 1f); // solid medium gray

        GUI.Box(new Rect(x, y, width, height), GUIContent.none); // empty box as background
        GUI.color = originalColor;

        GUIStyle toggleStyle = new GUIStyle(GUI.skin.toggle)
        {
            alignment = TextAnchor.MiddleCenter,
            fontSize = Mathf.RoundToInt(height * titleSize),
            normal = { textColor = Color.white },
            onNormal = { textColor = Color.green },
            hover = { textColor = Color.yellow }
        };

        string label = currentState ? labelOn : labelOff;
        return GUI.Toggle(new Rect(x, y, width, height), currentState, label, toggleStyle);
    }


    void DrawScreenTitle(float x, float y, float width, float height, string message) 
    {
        GUIStyle labelStyle = new GUIStyle(GUI.skin.label)
        {
            alignment = TextAnchor.MiddleCenter,
            fontSize = Mathf.RoundToInt(height * titleSize),
            normal = { textColor = Color.white }
        };
        GUI.Label(new Rect(x, y - height * titlePosition, width, height), message, labelStyle);
    }


    bool DrawScreenButton(float xP, float yP, float wP, float hP, string text, Action onClick)
    {
        float x = Screen.width * Mathf.Clamp01(xP);
        float y = Screen.height * Mathf.Clamp01(yP);
        float width = Screen.width * Mathf.Clamp01(wP);
        float height = Screen.height * Mathf.Clamp01(hP);

        GUIStyle buttonStyle = new GUIStyle(GUI.skin.button)
        {
            alignment = TextAnchor.MiddleCenter,
            fontSize = Mathf.RoundToInt(height * titleSize),
            normal = { textColor = Color.white },
            hover = { textColor = Color.yellow }
        };

        bool clicked = GUI.Button(new Rect(x, y, width, height), text, buttonStyle); 
        if (clicked)
        {
            onClick?.Invoke();
        }

        return clicked;
    }


/*
    void DrawScreenSlider(float xP, float yP, float wP, float hP, float min = 0f, float max = 1f)
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
*/


} // SliderController

} // SpicyTech5 
//-----------------------------------------------------------------------//