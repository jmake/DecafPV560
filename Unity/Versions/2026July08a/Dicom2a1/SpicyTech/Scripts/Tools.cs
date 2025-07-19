using System;
using System.Collections;
using System.Collections.Generic;
using System.Threading.Tasks;
using System.Diagnostics;

using UnityEngine;
using UnityEngine.UI;

using SpicyTech; 


namespace SpicyTech {


    public static 
    class Measure
    {
        public static void ExecutionTime(Action action, string label = "Execution Time")
        {
            Stopwatch stopwatch = Stopwatch.StartNew();
            action();
            stopwatch.Stop();

            UnityEngine.Debug.Log($"{label}: {stopwatch.ElapsedMilliseconds/1000.0} seg");
        }
    }


    public 
    class OnGUIThread : MonoBehaviour
    {
        public string message = "";

        [HideInInspector]
        public bool isProcessing = false;

        Action onRun; 

        public async 
        void Apply(Action function, string message, Action onComplete = null, Action onRun=null)
        {
            this.message = message;
            this.onRun = onRun;

            isProcessing = true;

            await Task.Yield();

            Func<Task> func = () => Task.Run(() => function()); 

            try
            {
                Stopwatch stopwatch = Stopwatch.StartNew();

                await func(); 
                //await Task.Delay(2000);

                stopwatch.Stop();
                UnityEngine.Debug.Log($"[OnGUIThread] {stopwatch.ElapsedMilliseconds/1000.0} seg ON '{message}' ");
            }
            catch (Exception ex)
            {
                UnityEngine.Debug.LogError($"Error during processing: {ex}");
            }
            onComplete?.Invoke();

            isProcessing = false;
            this.message = "";
            this.onRun = null;

            Destroy(this); // Auto-remove component after execution
        }


        public void Apply() 
        {
            UnityEngine.Debug.Log($"[OnGUIThread] ");
        }


        void OnGUI()
        {
            if (isProcessing)
            {
                DrawCenteredLabel(message, 0.25f, 0.2f); 
                if(this.onRun != null) this.onRun();
            }
        }


        //public  
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

    }


    public 
    class OnGUICoroutine : MonoBehaviour
    {
        string message = ""; 
        bool isProcessing = false;


        public  
        void Apply(Action function, string message) 
        {
            this.message = message; 
            StartCoroutine( ExecuteFunc(function) );
        }


        public  
        IEnumerator ExecuteFunc( Action function )
        {
            isProcessing = true;
            function(); 
            yield return null;
            isProcessing = false;
        }


        void OnGUI()
        {
            //Debug.Log("OnGUI called, isProcessing=" + isProcessing);

            if (isProcessing)
            {
                DrawCenteredLabel(message, 0.25f, 0.1f); // 25% width, 10% height
                isProcessing = false; 
            }
        } 


        public  
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


        public  
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

    }


} // SpicyTech