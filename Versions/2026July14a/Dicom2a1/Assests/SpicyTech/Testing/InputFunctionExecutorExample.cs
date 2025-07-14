using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;


public class InputFunctionExecutorExample : MonoBehaviour
{
    InputFunctionExecutor executor;

    void Start()
    {
        executor = gameObject.AddComponent<InputFunctionExecutor>();

        executor.SetFunction((parameters) =>
        {
            if (parameters.TryGetValue("key", out var key))
            {
                Debug.Log($"Function executed with key: {key}");
                // Simulate long operation:
                System.Threading.Thread.Sleep(1000);
            }
        });
    }
}
