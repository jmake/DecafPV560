using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;



public class InputFunctionExecutor : MonoBehaviour
{
    // Queue of parameter dictionaries
    private Queue<Dictionary<string, object>> inputQueue = new Queue<Dictionary<string, object>>();

    // Indicates if currently executing func
    private bool isProcessing = false;

    // The user-supplied function to execute
    private Action<Dictionary<string, object>> func;

    // Public method to set the function
    public void SetFunction(Action<Dictionary<string, object>> function)
    {
        func = function;
    }

    void Update()
    {
        // Example input detection - replace or expand as needed
        if (Event.current == null) // to avoid error when using Event.current inside Update
        {
            if (Input.GetKeyDown(KeyCode.X))
            {
                var param = new Dictionary<string, object> { { "key", "x" } };
                inputQueue.Enqueue(param);
            }
            if (Input.GetKeyDown(KeyCode.Y))
            {
                var param = new Dictionary<string, object> { { "key", "y" } };
                inputQueue.Enqueue(param);
            }
        }

        if (!isProcessing && inputQueue.Count > 0 && func != null)
        {
            var param = inputQueue.Dequeue();
            StartCoroutine(ExecuteFunc(param));
        }
    }

    IEnumerator ExecuteFunc(Dictionary<string, object> param)
    {
        isProcessing = true;
        func(param);
        // Simulate work with wait (remove if func is synchronous and fast)
        yield return null;
        isProcessing = false;
    }

    void OnGUI()
    {
        //Debug.Log("OnGUI called, isProcessing=" + isProcessing);

//        float currentProgress = 0.85f; // example progress value 0..1
        if (isProcessing)
        {
            DrawCenteredLabel("Processing...", 0.25f, 0.1f); // 25% width, 10% height
            //DrawCenteredStatusBar("Processing...", currentProgress, 0.4f, 0.05f);
        }

    }


    void DrawCenteredLabel(string text, float widthPercent, float heightPercent)
    {
        // Clamp between 0 and 1
        widthPercent = Mathf.Clamp01(widthPercent);
        heightPercent = Mathf.Clamp01(heightPercent);

        float width = Screen.width * widthPercent;
        float height = Screen.height * heightPercent;

        float x = (Screen.width - width) / 2f;
        float y = (Screen.height - height) / 2f;

        GUIStyle style = new GUIStyle(GUI.skin.label);
        style.alignment = TextAnchor.MiddleCenter;
        style.fontSize = Mathf.RoundToInt(height * 0.3f); // scalable font size

        GUI.Label(new Rect(x, y, width, height), text, style);
    }

/*
    void DrawCenteredStatusBar(string label, float progress, float widthPercent, float heightPercent)
    {
        // Clamp input
        progress = Mathf.Clamp01(progress);
        widthPercent = Mathf.Clamp01(widthPercent);
        heightPercent = Mathf.Clamp01(heightPercent);

        float width = Screen.width * widthPercent;
        float height = Screen.height * heightPercent;

        float x = (Screen.width - width) / 2f;
        float y = (Screen.height - height) / 2f;

        // Background box
        GUI.Box(new Rect(x, y, width, height), GUIContent.none);

        // Filled progress area
        float fillWidth = width * progress;
        GUI.Box(new Rect(x, y, fillWidth, height), GUIContent.none);

        // Label style
        GUIStyle style = new GUIStyle(GUI.skin.label);
        style.alignment = TextAnchor.MiddleCenter;
        style.fontSize = Mathf.RoundToInt(height * 0.6f);
        style.normal.textColor = Color.white;

        // Draw centered label on top
        GUI.Label(new Rect(x, y, width, height), label, style);
    }
*/

}