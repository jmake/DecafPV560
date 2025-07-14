using UnityEngine;
using System.Collections.Generic;


namespace SpicyTech {


public class CubeController : MonoBehaviour
{
    public GameObject plane; 
    ObjectSelector selector; 

    public Vector3 normalPrevious; 
    public Vector3 normalCurrent; 
    public GameObject cube; 

    Quaternion q0; 
    Vector3 normal0;

    Vector3 euler1;  
    Vector3 euler0;  
    Vector3 delta1;  

    public void Finish()
    {

    }


    public bool Apply()
    {
        bool moving = selector.StatusGet(); 
        //plane.SetActive(moving);

        if(moving) 
        {
            Quaternion q1 = cube.transform.rotation; 
            Quaternion delta = Quaternion.Inverse(q0) * q1;
            normalCurrent = delta * normal0; 

            euler0 = q0.eulerAngles;
            euler1 = q1.eulerAngles;
            delta1 = delta.eulerAngles; 
            Debug.Log($"[CubeController] delta : {delta1} [{euler0}->{euler1}] ");
        }

        return moving;
    }


    public GameObject Create(string name, Quaternion rotation) 
    {
        cube = GameObject.CreatePrimitive(PrimitiveType.Cube);
        cube.name = name + "_CubeController";
        cube.transform.position = Vector3.zero; 
        cube.transform.localScale = Vector3.one * 20.0f; 

        cube.GetComponent<Renderer>().material = new Material(Shader.Find("Custom/FaceColorByNormal"));
        AddLetter(cube, "X", new Vector3(0.501f,     0f,     0f), Quaternion.Euler(0, -90, 0) , 0.5f);
        AddLetter(cube, "Y", new Vector3(    0f, 0.501f,     0f), Quaternion.Euler(-90, 0, 0), 0.5f);
        AddLetter(cube, "Z", new Vector3(    0f,     0f, 0.501f), Quaternion.identity        , 0.5f);

        // reference plane 
        plane = new GameObject();
        plane.name = name + "_PlaneReference"; 
        plane.transform.localRotation = Quaternion.Euler(90, 0, 0); //rotation;  
        plane.transform.SetParent( cube.transform, true); // Set as child 
        plane.AddComponent<TransparentPlaneWithEdges>(); 
        plane.SetActive(false);

        cube.transform.rotation = rotation; // Here in order to apply to cube and plane correctly 

        selector = cube.AddComponent<ObjectSelector>(); 

        // - transform.up;      // Y-axis (0,1,0)
        // - transform.right;   // X-axis (1,0,0)
        // - transform.forward; // Z-axis (0,0,1) (default normal)
        normalCurrent = cube.transform.forward;
        normalPrevious = normalCurrent; // Avoid recalculation at the beginning 

        q0 = cube.transform.rotation; 
        normal0 = cube.transform.forward; 
        return cube; 
    }


    void AddLetter(GameObject parent, string letter, Vector3 localPos, Quaternion localRot, float characterSize)
    {
        var textObj = new GameObject(letter);
        textObj.transform.parent = parent.transform;
        textObj.transform.localPosition = localPos;
        textObj.transform.localRotation = localRot;

        var text = textObj.AddComponent<TextMesh>();
        text.text = letter;
        text.fontSize = 100;
        text.characterSize = characterSize; // letter size
        text.alignment = TextAlignment.Center;
        text.anchor = TextAnchor.MiddleCenter;
    }

} // CubeController


public class ObjectSelector : MonoBehaviour
{
    Selector selector = null; 
    Highlighter highlighter = null; 
    KeyboardInput kbInput = null;
    KeyboardController kbController = null;

    bool status; 
    string axis; 


    void Start()
    {
        selector = gameObject.AddComponent<Selector>();
        highlighter = gameObject.AddComponent<Highlighter>();

        kbInput = gameObject.AddComponent<KeyboardInput>();
        kbController = gameObject.AddComponent<KeyboardController>();

        axis = "y"; 
    }


    void Update()
    {
        status = false; 

        GameObject selected = selector.Apply(); 
        if(selected)
        {
            if (!string.IsNullOrEmpty(kbInput.keyPressed))
            {
                axis = kbInput.keyPressed; 
                Debug.Log($"[ObjectSelector] Key Pressed: '{axis}' ");
            }

            //Debug.Log($"[ObjectSelector] selected : '{selected.name}' ");
            status = kbController.Apply(selected, axis); 
        }

    }


    public bool StatusGet() 
    {
        return status; 
    }

} // ObjectSelector


public class KeyboardInput : MonoBehaviour
{
    public string keyPressed = null;


    public string GetKeyPressed()
    {
        Event e = Event.current; // only works inside OnGUI

        keyPressed = null; 
        if (e != null && e.type == EventType.KeyDown)
        {
            keyPressed = MapKey(e);
        }

        return keyPressed;
    }

    private string MapKey(Event e)
    {
        // Handle letters and numbers directly
        if (e.character >= 32 && e.character <= 126)
        {
            return e.character.ToString();
        }

        // Optional: map known special keys
        Dictionary<KeyCode, string> specialKeys = new Dictionary<KeyCode, string>
        {
            { KeyCode.Space, " " },
            { KeyCode.Tab, "\t" },
            { KeyCode.Return, "\n" },
            { KeyCode.KeypadEnter, "\n" },
            { KeyCode.BackQuote, "`" },
            { KeyCode.Minus, "-" },
            { KeyCode.Equals, "=" },
            { KeyCode.LeftBracket, "[" },
            { KeyCode.RightBracket, "]" },
            { KeyCode.Backslash, "\\" },
            { KeyCode.Semicolon, ";" },
            { KeyCode.Quote, "'" },
            { KeyCode.Comma, "," },
            { KeyCode.Period, "." },
            { KeyCode.Slash, "/" }
        };

        if (specialKeys.TryGetValue(e.keyCode, out string value))
        {
            return value;
        }

        return null;
    }

    void OnGUI()
    {
        GetKeyPressed(); 
    }

} // KeyboardInput


class HighlighterBase : MonoBehaviour {

    public GameObject LookingForObject(Camera c)
    {
        Ray ray = c.ScreenPointToRay(Input.mousePosition);

        GameObject targetObject = null; 
        if (Physics.Raycast(ray, out RaycastHit hit))
        {
            targetObject = hit.collider.gameObject;
        }

        return targetObject; 
    }


    public Material HighlightObject(GameObject obj, Color newColor)
    {
        Material mat = null; 
        if(!obj) return mat; 

        Renderer renderer = obj.GetComponent<Renderer>();
        if (renderer != null)
        {
            mat = renderer.material;
            
            Material highlightMat = new Material(mat);
            highlightMat.color = newColor; 
            renderer.material = highlightMat;
        }

        return mat; 
    }


    public void Reset(ref GameObject obj, ref Material mat)
    {
        if (obj != null)
        {
            Renderer renderer = obj.GetComponent<Renderer>();
            if (renderer != null && mat != null)
            {
                renderer.material = mat;
            }

            obj = null;
            mat = null;
        }
    }

}


class Highlighter : HighlighterBase {

    Color color; 
    Material material;
    GameObject selected;

    void Start() 
    {
        color = Color.yellow; 
        selected = null; 
        material = null; 
    }


    public GameObject Apply( )
    {
        base.Reset(ref selected, ref material); 
        selected = base.LookingForObject(Camera.main); 
        material = base.HighlightObject(selected, color);
        return selected; 
    }

} //Highlighter


class Selector : HighlighterBase {

    Color color; 
    Material material;
    GameObject selected;


    void Start() 
    {
        color = Color.red; 
        material = null; 
        selected = null; 
    }


    public GameObject Apply( )
    {
        MouseManager(); 
        return selected; 
    }


    void MouseManager() 
    {
        if( Input.GetMouseButtonDown(0) )
        {
            base.Reset(ref selected, ref material); 
            GameObject found = base.LookingForObject(Camera.main); 

            if(!found) return ;

            if(found != gameObject) return ; 

            material = base.HighlightObject(found, color);
            selected = found; 
        }
    }

} 


public class KeyboardController : MonoBehaviour
{
    public float moveSpeed = 10.0f * 2;
    public float rotateSpeed = 90.0f * 2;
    bool status; 


    public bool Apply(GameObject target, string plane)
    {
        status = false; 

        Transform t = target.transform;

        // Define movement and rotation axes based on selected plane
        Vector3 moveAxis = Vector3.zero;
        Vector3 rotateAxis = Vector3.zero;

        switch (plane.ToLower())
        {
            case "z":
                moveAxis = -t.up; 
                rotateAxis = t.forward;            
                break;

            case "y":
                moveAxis = -t.right;      
                rotateAxis = t.up;        
                break;

            case "x":
                moveAxis = -t.forward; 
                rotateAxis = t.right;  
                break;

            default:
                Debug.LogWarning("Unsupported plane: " + plane);
                return status; 
        }

        // Movement: Left / Right Arrow
        float moveInput = 0f;
        if (Input.GetKey(KeyCode.RightArrow)) moveInput = 1f;
        else if (Input.GetKey(KeyCode.LeftArrow)) moveInput = -1f;

        if (moveInput != 0f)
        {
            t.position += moveAxis.normalized * moveInput * moveSpeed * Time.deltaTime;
            status = true; 
        }

        // Rotation: Up / Down Arrow
        float rotateInput = 0f;
        if (Input.GetKey(KeyCode.UpArrow)) rotateInput = 1f;
        else if (Input.GetKey(KeyCode.DownArrow)) rotateInput = -1f;

        if (rotateInput != 0f)
        {
            t.Rotate(rotateAxis.normalized, rotateInput * rotateSpeed * Time.deltaTime, Space.World);
            status = true; 
        }

        return status;
    }

} // KeyboardController

} // SpicyTech