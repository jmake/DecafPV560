using UnityEditor;
using UnityEngine;


[CustomEditor(typeof(Plugging4))]
public class FileSelectorEditor : Editor
{
    public override void OnInspectorGUI()
    {
        var selector = (Plugging4)target;

        selector.selectedIndex = EditorGUILayout.Popup(
            "Select File",
            selector.selectedIndex,
            selector.fileOptions
        );

        EditorUtility.SetDirty(selector);
    }
}