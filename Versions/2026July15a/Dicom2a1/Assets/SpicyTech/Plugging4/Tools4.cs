using System;
using System.Collections;
using System.Collections.Generic;
using System.Threading.Tasks;
using System.Diagnostics;

using UnityEngine;
using UnityEngine.UI;

using SpicyTech; 


namespace SpicyTech4 {

    //-----------------------------------------------------------------------//
    //-----------------------------------------------------------------------//
    public 
    class OnGUIThread4 : MonoBehaviour
    {
        public string message = "";

        public float dtime = 0.0f;

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

                dtime = stopwatch.ElapsedMilliseconds / 1000.0f; 
                UnityEngine.Debug.Log($"[OnGUIThread] {dtime} seg ON '{message}' ");
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
                DrawCenteredLabel(message, 0.35f, 0.2f); 
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

    } // OnGUIThread4


    //-----------------------------------------------------------------------//
    //-----------------------------------------------------------------------//


    //-----------------------------------------------------------------------//
} // SpicyTech4