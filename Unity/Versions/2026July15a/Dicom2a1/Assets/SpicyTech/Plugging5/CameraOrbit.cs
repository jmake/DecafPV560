using UnityEngine;


namespace SpicyTech5 {
//-----------------------------------------------------------------------//

public class CameraOrbit : MonoBehaviour
{
    public Camera cam;            // Assign your Camera here
    public Transform target;      // Target object

    public float panSpeed = 100f;
    public float zoomSpeed = 10f;
    public float lookSpeed = 50f;

    private float distance;
    private Vector3 angles;

    void Start()
    {
        if (cam == null || target == null)
        {
            Debug.LogError("Camera or Target not assigned.");
            enabled = false;
            return;
        }

        distance = Vector3.Distance(cam.transform.position, target.position);
        angles = cam.transform.eulerAngles;
    }

    void LateUpdate()
    {
        if (target == null) return;

        float verticalInput = Input.GetAxis("Vertical");

        if (Input.GetKey(KeyCode.LeftShift))
        {
            // Only apply zoom if vertical input is non-zero
            if (Mathf.Abs(verticalInput) > 0.01f)
            {
                distance -= verticalInput * zoomSpeed;
                distance = Mathf.Clamp(distance, 1f, 1000f);
            }
        }
        else
        {
            // LookAt (tilt up/down)
            angles.x -= verticalInput * lookSpeed * Time.deltaTime;
            angles.x = Mathf.Clamp(angles.x, -80f, 80f);
        }

        // Pan (rotate around target Y axis)
        float panInput = Input.GetAxis("Horizontal"); // A/D or Left/Right arrows
        angles.y += panInput * panSpeed * Time.deltaTime;

        Quaternion rotation = Quaternion.Euler(angles.x, angles.y, 0);
        Vector3 offset = rotation * new Vector3(0, 0, -distance);

        cam.transform.position = target.position + offset;
        cam.transform.LookAt(target.position);
    }
}

} // SpicyTech5 
//-----------------------------------------------------------------------//